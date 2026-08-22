#include "../preproc-internals.h"

#include <inttypes.h>

bool format_value(ElStringBuf* out, ElPpValue* value) {
    switch (value->type) {
    case EL_PP_TYPE_NULL:
        return el_strbuf_append(out, EL_SV("null"));

    case EL_PP_TYPE_INT:
        return el_strbuf_appendf(out, "%"PRId64, value->as.int_);
    case EL_PP_TYPE_FLOAT:
        return el_strbuf_appendf(out, "%g", value->as.float_);
    case EL_PP_TYPE_CHAR:
        return el_strbuf_appendf(out, "%c", value->as.char_);

    case EL_PP_TYPE_BOOL:
        return el_strbuf_append_cstr(out, value->as.bool_ ? "true" : "false");
    case EL_PP_TYPE_STR:
        return el_strbuf_append(out, value->as.str_);
    case EL_PP_TYPE_TOK:
        return el_token_to_raw_string(&value->as.tok_, out, false);

    case EL_PP_TYPE_LIST:
        el_strbuf_append(out, EL_SV("{ "));

        ElPpList l = value->as.list_;
        for (ElPpValue** v = l.values; v < l.values + l.count; ++v) {
            if (v != l.values)
                el_strbuf_append(out, EL_SV(", "));
            format_value(out, *v);
        }

        el_strbuf_append(out, EL_SV(" }"));
        return true;
    }

    EL_UNREACHABLE_ENUM_VAL(ElPpType, value->type);
}

bool _el_pp_handle_diag(ElPreproc* pp, ElDiagSeverity sev, ElSourceSpan dspan) {
    ElStringBuf message;
    el_strbuf_init(&message);

    while (true) {
        ElPpValue* value = _el_pp_eval(pp);
        if (value == NULL) {
            return false;
        }

        if (!format_value(&message, value))
            return false;

        if (!_el_pp_match(pp, EL_TT_COMMA)) break;

        el_strbuf_append_char(&message, ' ');
    }

    el_diag_report_nocat(
        pp->diag, sev, dspan, "${message}",
        EL_DIAG_STRING("message", el_strbuf_view(&message)),
    );

    el_strbuf_destroy(&message);
    return sev != EL_DIAG_ERROR;
}

bool _el_pp_skip_diag(ElPreproc* pp) {
    while (true) {
        if (!_el_pp_skip_expr(pp)) return false;
        if (!_el_pp_match(pp, EL_TT_COMMA)) break;
    }
    return true;
}
