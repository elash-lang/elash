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

bool _el_pp_preprocess_directive(ElPreproc* pp, ElToken hash, ElToken* out_tok) {
    ElToken dir;
    if (!_el_pp_read(pp, &dir)) {
        report_unexpected_eof(pp, hash);
        return false;
    }

    ElSourceSpan dspan =
        el_srcspan_merge(hash.span, dir.span);

    if (el_sv_eql(dir.lexeme, EL_SV("include")))
        return _el_pp_handle_include(pp, dspan) && _el_pp_next_d(pp, out_tok);
    if (el_sv_eql(dir.lexeme, EL_SV("emit")))
        return _el_pp_handle_emit(pp, dspan) && _el_pp_next_d(pp, out_tok);
    if (el_sv_eql(dir.lexeme, EL_SV("embed")))
        return _el_pp_handle_embed(pp, dspan) && _el_pp_next_d(pp, out_tok);

    if (el_sv_eql(dir.lexeme, EL_SV("const")))
        return _el_pp_handle_const(pp, dspan) && _el_pp_next_d(pp, out_tok);
    if (el_sv_eql(dir.lexeme, EL_SV("var")))
        return _el_pp_handle_var(pp, dspan) && _el_pp_next_d(pp, out_tok);
    if (el_sv_eql(dir.lexeme, EL_SV("set")))
        return _el_pp_handle_set(pp, dspan) && _el_pp_next_d(pp, out_tok);
    if (el_sv_eql(dir.lexeme, EL_SV("inc")))
        return _el_pp_handle_inc(pp, dspan) && _el_pp_next_d(pp, out_tok);
    if (el_sv_eql(dir.lexeme, EL_SV("dec")))
        return _el_pp_handle_dec(pp, dspan) && _el_pp_next_d(pp, out_tok);

    if (el_sv_eql(dir.lexeme, EL_SV("if")))
        return _el_pp_handle_if(pp, dspan) && _el_pp_next_d(pp, out_tok);
    if (el_sv_eql(dir.lexeme, EL_SV("else")))
        return _el_pp_handle_else(pp, dspan) && _el_pp_next_d(pp, out_tok);
    if (el_sv_eql(dir.lexeme, EL_SV("elif")))
        return _el_pp_handle_elif(pp, dspan) && _el_pp_next_d(pp, out_tok);
    if (el_sv_eql(dir.lexeme, EL_SV("end")))
        return _el_pp_handle_end(pp, dspan) && _el_pp_next_d(pp, out_tok);

    if (el_sv_eql(dir.lexeme, EL_SV("error")))
        return _el_pp_handle_diag(pp, EL_DIAG_ERROR, dspan) && _el_pp_next_d(pp, out_tok);
    if (el_sv_eql(dir.lexeme, EL_SV("note")))
        return _el_pp_handle_diag(pp, EL_DIAG_NOTE, dspan) && _el_pp_next_d(pp, out_tok);
    if (el_sv_eql(dir.lexeme, EL_SV("warn")))
        return _el_pp_handle_diag(pp, EL_DIAG_WARN, dspan) && _el_pp_next_d(pp, out_tok);

    report_unknown_dir(pp, dspan, dir);
    return false;
}

bool _el_pp_skip_directive(ElPreproc* pp, ElToken hash) {
    ElToken dir;
    if (!_el_pp_read(pp, &dir)) {
        report_unexpected_eof(pp, hash);
        return false;
    }

    ElSourceSpan dspan =
        el_srcspan_merge(hash.span, dir.span);

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
    if (el_sv_eql(dir.lexeme, EL_SV("end")))   return _el_pp_skip_end(pp);

    // theoretically we could just skip unknown directives,
    // but maybe it's better to validate them for catching
    // errors faster etc.
    report_unknown_dir(pp, dspan, dir);
    return false;
}
