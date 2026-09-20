#include "preproc-internals.h"

static void report_unknown_dir(ElPreproc* pp, ElSourceSpan dspan, ElToken dir) {
    el_diag_report(
        pp->diag, EL_DIAG_ERROR, "pp.unknown-dir",
        dspan, "unknown dir: #${name}",
        EL_DIAG_STRING("name", dir.lexeme),
    );
}

static void report_unexpected_eof(ElPreproc* pp, ElToken hash) {
    el_diag_report(
        pp->diag, EL_DIAG_ERROR, "pp.unexpected-token",
        hash.span, "unexpected end of input after '#'"
    );
}

static bool report_func_token_dir(ElPreproc* pp, ElSourceSpan dspan, ElStringView name) {
    return el_diag_report(
        pp->diag, EL_DIAG_ERROR, "pp.func-emit",
        dspan, "cannot use #${name} inside a function",
        EL_DIAG_STRING("name", name),
    );
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity): the logic is flat
static bool _preprocess_directive_internal(ElPreproc* pp, ElToken hash, ElToken* out_tok) {
    ElToken dir;
    if (!_el_pp_read(pp, &dir)) {
        report_unexpected_eof(pp, hash);
        return false;
    }

    ElSourceSpan dspan =
        el_srcspan_merge(hash.span, dir.span);

    pp->operation_count += EL_PP_DIR_OPS;
    if (!_el_pp_ensure_ops_available(pp, dspan))
        return false;

    if (el_sv_eql(dir.lexeme, EL_SV("include")))
        return _el_pp_handle_include(pp, dspan);
    if (el_sv_eql(dir.lexeme, EL_SV("emit")))
        return _el_pp_handle_emit(pp, dspan);
    if (el_sv_eql(dir.lexeme, EL_SV("embed")))
        return _el_pp_handle_embed(pp, dspan);

    if (el_sv_eql(dir.lexeme, EL_SV("const")))
        return _el_pp_handle_const(pp, dspan);
    if (el_sv_eql(dir.lexeme, EL_SV("var")))
        return _el_pp_handle_var(pp, dspan);
    if (el_sv_eql(dir.lexeme, EL_SV("set")))
        return _el_pp_handle_set(pp, dspan);
    if (el_sv_eql(dir.lexeme, EL_SV("inc")))
        return _el_pp_handle_inc(pp, dspan);
    if (el_sv_eql(dir.lexeme, EL_SV("dec")))
        return _el_pp_handle_dec(pp, dspan);

    if (el_sv_eql(dir.lexeme, EL_SV("if")))
        return _el_pp_handle_if(pp, dspan);
    if (el_sv_eql(dir.lexeme, EL_SV("else")))
        return _el_pp_handle_else(pp, dspan);
    if (el_sv_eql(dir.lexeme, EL_SV("elif")))
        return _el_pp_handle_elif(pp, dspan);
    if (el_sv_eql(dir.lexeme, EL_SV("end")))
        return _el_pp_handle_end(pp, dspan);

    if (el_sv_eql(dir.lexeme, EL_SV("func")))
        return _el_pp_handle_func(pp, dspan);
    if (el_sv_eql(dir.lexeme, EL_SV("return"))) {
        if (!_el_pp_handle_return(pp, dspan)) return false;
        *out_tok = (ElToken) { .type = EL_TT_EOF };
        return true;
    }

    if (el_sv_eql(dir.lexeme, EL_SV("while")))
        return _el_pp_handle_while(pp, dspan);
    if (el_sv_eql(dir.lexeme, EL_SV("for")))
        return _el_pp_handle_for(pp, dspan);

    if (el_sv_eql(dir.lexeme, EL_SV("error")))
        return _el_pp_handle_diag(pp, EL_DIAG_ERROR, dspan);
    if (el_sv_eql(dir.lexeme, EL_SV("note")))
        return _el_pp_handle_diag(pp, EL_DIAG_NOTE, dspan);
    if (el_sv_eql(dir.lexeme, EL_SV("warn")))
        return _el_pp_handle_diag(pp, EL_DIAG_WARN, dspan);

    report_unknown_dir(pp, dspan, dir);
    return false;
}

bool _el_pp_preprocess_directive(ElPreproc* pp, ElToken hash, ElToken* out_tok) {
    el_prof_begin_sub(pp->prof, pp->pss_directive);
    bool result = _preprocess_directive_internal(pp, hash, out_tok);
    el_prof_finish_sub(pp->prof, pp->pss_directive);
    return result;
}

bool _el_pp_skip_directive(ElPreproc* pp, ElToken hash) {
    bool capturing = pp->skip_capture;

    // it may be an #end directive which we definitely
    // don't want to capture to the buffer
    pp->skip_capture = false;

    ElToken dir;
    if (!_el_pp_read(pp, &dir)) {
        pp->skip_capture = capturing;
        report_unexpected_eof(pp, hash);
        return false;
    }

    ElSourceSpan dspan =
        el_srcspan_merge(hash.span, dir.span);

    bool is_end = el_sv_eql(dir.lexeme, EL_SV("end"));
    if (is_end && capturing && pp->skip_depth == 1) {
        return _el_pp_skip_end(pp);
    }

    if (capturing) {
        if (!el_tkbuf_push(&pp->capture_buf, hash)) return false;
        if (!el_tkbuf_push(&pp->capture_buf, dir))  return false;
        pp->skip_capture = true;
    }

    bool in_func_body =
        pp->call_stack != NULL
        || (capturing
            && pp->block_stack != NULL
            && pp->block_stack->kind == EL_PP_BLOCK_FUNC);

    if (in_func_body) {
        if (el_sv_eql(dir.lexeme, EL_SV("emit")) || el_sv_eql(dir.lexeme, EL_SV("include"))) {
            report_func_token_dir(pp, dspan, dir.lexeme);
            //return false;
        }
    }

    if (el_sv_eql(dir.lexeme, EL_SV("include"))) return _el_pp_skip_include(pp);
    if (el_sv_eql(dir.lexeme, EL_SV("emit")))    return _el_pp_skip_emit(pp);
    if (el_sv_eql(dir.lexeme, EL_SV("embed")))   return _el_pp_skip_embed(pp);

    if (el_sv_eql(dir.lexeme, EL_SV("const"))) return _el_pp_skip_const(pp);
    if (el_sv_eql(dir.lexeme, EL_SV("var")))   return _el_pp_skip_var(pp);
    if (el_sv_eql(dir.lexeme, EL_SV("set")))   return _el_pp_skip_set(pp);
    if (el_sv_eql(dir.lexeme, EL_SV("inc")))   return _el_pp_skip_incdec(pp);
    if (el_sv_eql(dir.lexeme, EL_SV("dec")))   return _el_pp_skip_incdec(pp);

    if (el_sv_eql(dir.lexeme, EL_SV("error"))) return _el_pp_skip_diag(pp);
    if (el_sv_eql(dir.lexeme, EL_SV("note")))  return _el_pp_skip_diag(pp);
    if (el_sv_eql(dir.lexeme, EL_SV("warn")))  return _el_pp_skip_diag(pp);

    if (el_sv_eql(dir.lexeme, EL_SV("if")))    return _el_pp_skip_if(pp);
    if (el_sv_eql(dir.lexeme, EL_SV("else")))  return _el_pp_skip_else(pp, dspan);
    if (el_sv_eql(dir.lexeme, EL_SV("elif")))  return _el_pp_skip_elif(pp, dspan);

    if (el_sv_eql(dir.lexeme, EL_SV("func")))   return _el_pp_skip_func(pp);
    if (el_sv_eql(dir.lexeme, EL_SV("return"))) return _el_pp_skip_return(pp);
    if (el_sv_eql(dir.lexeme, EL_SV("while")))  return _el_pp_skip_while(pp);
    if (el_sv_eql(dir.lexeme, EL_SV("for")))    return _el_pp_skip_for(pp);

    if (is_end) return _el_pp_skip_end(pp);

    // theoretically we could just skip unknown directives,
    // but maybe it's better to validate them for catching
    // errors faster etc.
    report_unknown_dir(pp, dspan, dir);
    return false;
}
