#include "preproc-internals.h"

#include <elash/sema/unary-op.h>
#include <elash/sema/bin-op.h>

void* _el_pp_report_incdec(ElPreproc* pp, ElSourceSpan span) {
    el_diag_report(
        pp->diag, EL_DIAG_ERROR, "pp.invalid-op", span,
        "incrementation/decrementation operators unsupported in preprocessor expressions",
    );
    el_diag_help(
        pp->diag, "you may want to use #inc/#dec directives"
    );
    return NULL;
}

void* _el_pp_report_deref(ElPreproc* pp, ElSourceSpan span) {
    el_diag_report(
        pp->diag, EL_DIAG_ERROR, "pp.invalid-op", span,
        "the dereference operator is unsupported in preprocessor expressions",
    );
    el_diag_help(
        pp->diag, "the elash's preprocessor doesn't support references",
    );
    return NULL;
}

void* _el_pp_report_float_bw(ElPreproc* pp, ElSourceSpan span, ElSemaBinOp op) {
    return el_diag_report(
        pp->diag, EL_DIAG_ERROR, "pp.invalid-op", span,
        "operator '${op}' is not defined for floats",
        EL_DIAG_STRING("op", el_sema_bin_op_to_string(op))
    );
}

void* _el_pp_report_non_bool_logical(ElPreproc* pp, ElSourceSpan span, ElSemaBinOp op) {
    return el_diag_report(
          pp->diag, EL_DIAG_ERROR, "pp.invalid-op", span,
          "logical operator '${op}' requires boolean operands",
          EL_DIAG_STRING("op", el_sema_bin_op_to_string(op)),
    );
}

void* _el_pp_report_non_bool_logical_unary(ElPreproc* pp, ElSourceSpan span, ElSemaUnaryOp op) {
    return el_diag_report(
          pp->diag, EL_DIAG_ERROR, "pp.invalid-op", span,
          "logical operator '${op}' requires a boolean operand",
          EL_DIAG_STRING("op", el_sema_unary_op_to_string(op)),
    );
}

void* _el_pp_report_unterm_quote(ElPreproc* pp, ElSourceSpan span) {
    return el_diag_report(
        pp->diag, EL_DIAG_ERROR, "pp.invalid-quote",
        span, "unterminated token list '#(...)'"
    );
}
