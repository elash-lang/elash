#include <elc/cli/argparse.h>
#include <elash/defs/sv.h>

static ElStringView get_value(ElcArgParseContext* p, ElStringView arg, ElStringView flag_name) {
    if (el_sv_starts_with(arg, flag_name) && arg.len > flag_name.len && arg.data[flag_name.len] == '=') {
        return el_sv_slice(arg, flag_name.len + 1, arg.len);
    }
    if (p->i + 1 < p->argc) {
        p->i++;
        return el_sv_from_cstr(p->argv[p->i]);
    }
    return EL_SV_NULL;
}

static ElStringView get_optional_path(ElcArgParseContext* p, ElStringView arg, ElStringView flag_name) {
    if (el_sv_starts_with(arg, flag_name) && arg.len > flag_name.len && arg.data[flag_name.len] == '=') {
        return el_sv_slice(arg, flag_name.len + 1, arg.len);
    }
    if (p->i + 1 < p->argc && p->argv[p->i + 1][0] != '-') {
        p->i++;
        return el_sv_from_cstr(p->argv[p->i]);
    }
    return el_sv_from_cstr("-");
}

static bool handle_dump_switch(ElcArgParseContext* p, ElStringView arg, ElStringView flag_name, ElcCliDumpSwitch* sw) {
    if (el_sv_starts_with(arg, flag_name)) {
        sw->is_enabled = true;
        sw->output = get_optional_path(p, arg, flag_name);
        return true;
    }
    return false;
}

static ElcCliParseResult handle_artifact_flag(
    ElcArgParseContext *p, ElStringView arg, ElStringView flag, ElcArtifactKind *out
) {
    ElStringView val = get_value(p, arg, flag);
    if (val.len == 0) {
        return (ElcCliParseResult) {
            .code = ELC_CLI_PARSE_EXPECTED_VALUE, .ctx.str = flag
        };
    }
    *out = elc_artifact_kind_from_string(val);
    if (*out == ELC_ART_NONE) {
        return (ElcCliParseResult) {
            .code = ELC_CLI_PARSE_UNKNOWN_ARTIFACT, .ctx.str = val
        };
    }
    return ELC_CLI_PARSE_RESULT_OK;
}

static ElcCliParseResult handle_output_flag(ElcArgParseContext* p, ElStringView arg) {
    p->out->output = get_value(p, arg, EL_SV("--output"));
    if (p->out->output.len == 0) {
        return (ElcCliParseResult) {
            .code = ELC_CLI_PARSE_EXPECTED_VALUE,
            .ctx.str = EL_SV("--output")
        };
    }
    return ELC_CLI_PARSE_RESULT_OK;
}

static ElcCliParseResult handle_until_flag(ElcArgParseContext *p, ElStringView arg) {
    return handle_artifact_flag(p, arg, EL_SV("--until"), &p->out->until);
}

static ElcCliParseResult handle_emit_flag(ElcArgParseContext *p, ElStringView arg) {
    return handle_artifact_flag(p, arg, EL_SV("--emit"), &p->out->emit);
}

static ElcOptLevel elc_opt_level_from_string(ElStringView s) {
    if (el_sv_eql(s, EL_SV("0")))     return ELC_OPT_O0;
    if (el_sv_eql(s, EL_SV("1")))     return ELC_OPT_O1;
    if (el_sv_eql(s, EL_SV("2")))     return ELC_OPT_O2;
    if (el_sv_eql(s, EL_SV("3")))     return ELC_OPT_O3;
    if (el_sv_eql(s, EL_SV("g")))     return ELC_OPT_Og;
    if (el_sv_eql(s, EL_SV("s")))     return ELC_OPT_Os;
    if (el_sv_eql(s, EL_SV("z")))     return ELC_OPT_Oz;
    if (el_sv_eql(s, EL_SV("f")))     return ELC_OPT_Of;
    if (el_sv_eql(s, EL_SV("debug"))) return ELC_OPT_Og;
    if (el_sv_eql(s, EL_SV("fast")))  return ELC_OPT_Of;
    if (el_sv_eql(s, EL_SV("size")))  return ELC_OPT_Os;
    return ELC_OPT_UNSPEC;
}

static ElcCliParseResult handle_opt_flag(ElcArgParseContext* p, ElStringView arg) {
    ElStringView val = get_value(p, arg, EL_SV("--opt"));
    if (val.len == 0) {
        return (ElcCliParseResult) {
            .code = ELC_CLI_PARSE_EXPECTED_VALUE,
            .ctx.str = EL_SV("--opt")
        };
    }
    ElcOptLevel level = elc_opt_level_from_string(val);
    if (level == ELC_OPT_UNSPEC) {
         return (ElcCliParseResult) {
             .code = ELC_CLI_PARSE_UNKNOWN_OPT_LEVEL,
             .ctx.str = val
         };
    }

    p->out->opt = level;
    return ELC_CLI_PARSE_RESULT_OK;
}

static ElcCliParseResult parse_preference_flag(
    ElcArgParseContext* p, ElStringView arg, ElStringView flag_name, ElcPreference* out
) {
    ElStringView val = get_value(p, arg, flag_name);
    if (val.len == 0)
        return (ElcCliParseResult) {
            .code = ELC_CLI_PARSE_EXPECTED_VALUE,
            .ctx.str = flag_name
        };

    if (el_sv_eql(val, EL_SV("never")))  { *out = ELC_PREF_NEVER;  return ELC_CLI_PARSE_RESULT_OK; }
    if (el_sv_eql(val, EL_SV("auto")))   { *out = ELC_PREF_AUTO;   return ELC_CLI_PARSE_RESULT_OK; }
    if (el_sv_eql(val, EL_SV("always"))) { *out = ELC_PREF_ALWAYS; return ELC_CLI_PARSE_RESULT_OK; }

    return (ElcCliParseResult) {
        .code = ELC_CLI_PARSE_UNKNOWN_PREFERENCE,
        .ctx.str = val
    };
}

static ElcCliParseResult handle_long_flag(ElcArgParseContext* p, ElStringView arg) {
    if (el_sv_eql(arg, EL_SV("--help")))    { p->out->help = true;    return ELC_CLI_PARSE_RESULT_OK; }
    if (el_sv_eql(arg, EL_SV("--version"))) { p->out->version = true; return ELC_CLI_PARSE_RESULT_OK; }

    if (el_sv_starts_with(arg, EL_SV("--output"))) {
        ElcCliParseResult err = handle_output_flag(p, arg);
        return err;
    }
    if (el_sv_starts_with(arg, EL_SV("--until"))) {
        ElcCliParseResult err = handle_until_flag(p, arg);
        return err;
    }
    if (el_sv_starts_with(arg, EL_SV("--emit"))) {
        ElcCliParseResult err = handle_emit_flag(p, arg);
        return err;
    }
    if (el_sv_starts_with(arg, EL_SV("--opt"))) {
        ElcCliParseResult err = handle_opt_flag(p, arg);
        return err;
    }
    if (el_sv_starts_with(arg, EL_SV("--color"))) {
        ElcCliParseResult err = parse_preference_flag(p, arg, EL_SV("--color"), &p->out->color);
        return err;
    }

    if (handle_dump_switch(p, arg, EL_SV("--dump-toks"),    &p->out->dump_toks))    return ELC_CLI_PARSE_RESULT_OK;
    if (handle_dump_switch(p, arg, EL_SV("--dump-pp-toks"), &p->out->dump_pp_toks)) return ELC_CLI_PARSE_RESULT_OK;
    if (handle_dump_switch(p, arg, EL_SV("--dump-ast"),     &p->out->dump_ast))     return ELC_CLI_PARSE_RESULT_OK;
    if (handle_dump_switch(p, arg, EL_SV("--dump-hir"),     &p->out->dump_hir))     return ELC_CLI_PARSE_RESULT_OK;
    if (handle_dump_switch(p, arg, EL_SV("--dump-mir"),     &p->out->dump_mir))     return ELC_CLI_PARSE_RESULT_OK;
    if (handle_dump_switch(p, arg, EL_SV("--dump-lir"),     &p->out->dump_lir))     return ELC_CLI_PARSE_RESULT_OK;
    if (handle_dump_switch(p, arg, EL_SV("--dump-asm"),     &p->out->dump_asm))     return ELC_CLI_PARSE_RESULT_OK;

    return (ElcCliParseResult) {
        .code = ELC_CLI_PARSE_UNKNOWN_LONG_FLAG,
        .ctx.str = arg
    };
}

static ElStringView get_short_value(ElcArgParseContext* p, ElStringView arg, usize j) {
    if (j + 1 < arg.len) {
        return el_sv_slice(arg, j + 1, arg.len);
    }
    if (p->i + 1 < p->argc) {
        p->i++;
        return el_sv_from_cstr(p->argv[p->i]);
    }
    return EL_SV_NULL;
}

static ElcCliParseResult handle_short_flag(ElcArgParseContext* p, ElStringView arg) {
    for (usize j = 1; j < arg.len; j++) {
        char c = arg.data[j];
        switch (c) {
        case 'h': p->out->help    = true; break;
        case 'v': p->out->version = true; break;
        case 'o': {
            ElStringView val = get_short_value(p, arg, j);
            if (val.len == 0) {
                return (ElcCliParseResult) { .code = ELC_CLI_PARSE_EXPECTED_VALUE, .ctx.str = EL_SV("-o") };
            }
            p->out->output = val;
            return ELC_CLI_PARSE_RESULT_OK;
        }
        case 'O': {
            ElStringView val = get_short_value(p, arg, j);
            if (val.len == 0)
                return (ElcCliParseResult) { .code = ELC_CLI_PARSE_EXPECTED_VALUE, .ctx.str = EL_SV("-O") };

            ElcOptLevel level = elc_opt_level_from_string(val);
            if (level == ELC_OPT_UNSPEC)
                return (ElcCliParseResult) { .code = ELC_CLI_PARSE_UNKNOWN_OPT_LEVEL, .ctx.str = val };

            p->out->opt = level;
            return ELC_CLI_PARSE_RESULT_OK;
        }
        default:
            return (ElcCliParseResult) {
                .code = ELC_CLI_PARSE_UNKNOWN_SHORT_FLAG,
                .ctx.c = c
            };
        }
    }
    return ELC_CLI_PARSE_RESULT_OK;
}

static ElcCliParseResult handle_pos_arg(ElcArgParseContext* p, ElStringView arg) {
    if (!p->cmd_set) {
        if (el_sv_eql(arg, EL_SV("help")))    { p->out->help = true;    p->cmd_set = true; return ELC_CLI_PARSE_RESULT_OK; }
        if (el_sv_eql(arg, EL_SV("version"))) { p->out->version = true; p->cmd_set = true; return ELC_CLI_PARSE_RESULT_OK; }

        static const struct {
            ElStringView name;
            ElcArtifactKind until;
            ElcArtifactKind emit;
        } aliases[] = {
            { EL_SV("build"),      ELC_ART_OBJ,       ELC_ART_OBJ },
            { EL_SV("compile"),    ELC_ART_OBJ,       ELC_ART_OBJ },
            { EL_SV("check"),      ELC_ART_HIR,       ELC_ART_NONE },
            { EL_SV("preprocess"), ELC_ART_PP_TOKENS, ELC_ART_PP_TOKENS },
            { EL_SV("lower"),      ELC_ART_MIR,       ELC_ART_MIR },
            { EL_SV("inspect"),    ELC_ART_OBJ,       ELC_ART_NONE },
        };
        static const usize aliases_count = sizeof(aliases) / sizeof(aliases[0]);

        for (usize k = 0; k < aliases_count; k++) {
            if (el_sv_eql(arg, aliases[k].name)) {
                p->out->until = aliases[k].until;
                p->out->emit = aliases[k].emit;
                p->cmd_set = true;
                return ELC_CLI_PARSE_RESULT_OK;
            }
        }
    }

    if (p->out->input.len > 0) {
        return (ElcCliParseResult) {
            .code = ELC_CLI_PARSE_MULTIPLE_INPUTS,
            .ctx.str = arg
        };
    }
    p->out->input = arg;
    return ELC_CLI_PARSE_RESULT_OK;
}

ElcCliParseResult elc_cli_parse_args(int argc, const char* const* argv, ElcArgs* out) {
    memset(out, 0, sizeof(ElcArgs));
    out->output = el_sv_from_cstr("-");
    out->opt = ELC_OPT_UNSPEC;
    out->until = ELC_ART_OBJ;
    out->emit = ELC_ART_OBJ;

    ElcArgParseContext p = {
        .argc = argc,
        .argv = argv,
        .i = 1,
        .stop_flags = false,
        .out = out,
        .cmd_set = false
    };

    for (; p.i < p.argc; p.i++) {
        ElStringView arg = el_sv_from_cstr(p.argv[p.i]);

        if (!p.stop_flags && el_sv_eql(arg, EL_SV("--"))) {
            p.stop_flags = true;
            continue;
        }

        if (!p.stop_flags && el_sv_starts_with(arg, EL_SV("--"))) {
            ElcCliParseResult res = handle_long_flag(&p, arg);
            if (res.code != ELC_CLI_PARSE_OK) return res;
        } else if (!p.stop_flags && el_sv_starts_with(arg, EL_SV("-")) && arg.len > 1) {
            ElcCliParseResult res = handle_short_flag(&p, arg);
            if (res.code != ELC_CLI_PARSE_OK) return res;
        } else {
            ElcCliParseResult res = handle_pos_arg(&p, arg);
            if (res.code != ELC_CLI_PARSE_OK) return res;
        }
    }

    if (!out->help && !out->version && out->input.len == 0) {
        return (ElcCliParseResult) { .code = ELC_CLI_PARSE_MISSING_INPUT };
    }

    return ELC_CLI_PARSE_RESULT_OK;
}
