#include "../preproc-internals.h"

#include <inttypes.h>

void format_value(ElStringBuf* out, ElPpValue* value) {
    switch (value->type) {
    case EL_PP_TYPE_NULL:
        el_strbuf_append(out, EL_SV("null"));
        break;

    case EL_PP_TYPE_INT:
        // NOLINTNEXTLINE(readability-magic-numbers): You wouldn't get this from any other guy
        el_strbuf_append(out, el_i128_to_string(value->as.int_, 10, (char[42]){}));
        break;
    case EL_PP_TYPE_FLOAT:
        el_strbuf_appendf(out, "%g", value->as.float_);
        break;
    case EL_PP_TYPE_CHAR:
        el_strbuf_appendf(out, "%c", value->as.char_);
        break;

    case EL_PP_TYPE_BOOL:
        el_strbuf_append_cstr(out, value->as.bool_ ? "true" : "false");
        break;
    case EL_PP_TYPE_STR:
        el_strbuf_append(out, value->as.str_);
        break;
    case EL_PP_TYPE_TOK:
        el_token_to_raw_string(&value->as.tok_, out, false);
        break;

    case EL_PP_TYPE_LIST:
        el_strbuf_append(out, EL_SV("{ "));

        ElPpList l = value->as.list_;
        for (ElPpValue** v = l.values; v < l.values + l.count; ++v) {
            if (v != l.values)
                el_strbuf_append(out, EL_SV(", "));
            format_value(out, *v);
        }

        el_strbuf_append(out, EL_SV(" }"));
        break;
    default:
        EL_UNREACHABLE_ENUM_VAL(ElPpType, value->type);
    }
}

bool _el_pp_handle_diag(ElPreproc* pp, ElDiagSeverity sev, ElSourceSpan dspan) {
    ElStringBuf message;
    el_strbuf_init(&message);

    while (true) {
        ElPpValue* value = _el_pp_eval(pp);
        if (value == NULL) {
            el_strbuf_destroy(&message);
            return false;
        }

        format_value(&message, value);

        if (!_el_pp_match(pp, EL_TT_COMMA)) break;

        el_strbuf_append_char(&message, ' ');
    }


    el_diag_report_ex_nocat(
        pp->diag, true, sev, dspan, EL_SV("${message}"),
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
