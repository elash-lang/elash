#include "../preproc-internals.h"

#include <elash/pp/value.h>

static bool eval_cond(ElPreproc* pp, ElSourceSpan dspan, bool* out) {
    ElPpValue* val = _el_pp_eval(pp);
    if (val == NULL) return false;

    if (val->type != EL_PP_TYPE_BOOL) {
        return el_diag_report(
            pp->diag, EL_DIAG_ERROR, "pp.if-type", dspan,
            "#if condition must be a boolean, got ${type}",
            EL_DIAG_STRING("type", _el_pp_type_name(val->type))
        );
    }

    *out = val->as.bool_;
    return true;
}

static bool check_else(ElPreproc* pp, ElSourceSpan dspan) {
    if (pp->if_stack == NULL) {
        return el_diag_report(
            pp->diag, EL_DIAG_ERROR, "pp.stray-else",
            dspan, "#else without matching #if"
        );
    }

    if (pp->if_stack->had_else) {
        return el_diag_report(
            pp->diag, EL_DIAG_ERROR, "pp.dup-else",
            dspan, "duplicated #else"
        );
    }

    pp->if_stack->had_else = true;
    return true;
}

bool _el_pp_handle_if(ElPreproc* pp, ElSourceSpan dspan) {
    bool cond;
    if (!eval_cond(pp, dspan, &cond)) {
        return false;
    }

    _el_pp_push_if_frame(pp, cond, dspan);
    if (!cond) {
        pp->skip_depth++;
    }
    return true;
}

bool _el_pp_handle_else(ElPreproc* pp, ElSourceSpan dspan) {
    if (!check_else(pp, dspan)) {
        return false;
    }

    EL_ASSERT(pp->if_stack->has_scope, "#else in active path without a scope");
    _el_pp_leave_if_branch(pp);

    pp->skip_depth++;
    return true;
}

bool _el_pp_handle_elif(ElPreproc* pp, ElSourceSpan dspan) {
    if (pp->if_stack == NULL) {
        return el_diag_report(
            pp->diag, EL_DIAG_ERROR, "pp.stray-elif",
            dspan, "#elif without matching #if"
        );
    }

    if (pp->if_stack->had_else) {
        return el_diag_report(
            pp->diag, EL_DIAG_ERROR, "pp.elif-after-else",
            dspan, "#elif after #else"
        );
    }

    if (!_el_pp_skip_expr(pp)) {
        return false;
    }

    pp->skip_depth++;
    return true;
}

bool _el_pp_handle_end(ElPreproc* pp, ElSourceSpan dspan) {
    if (pp->if_stack == NULL) {
        return el_diag_report(
            pp->diag, EL_DIAG_ERROR, "pp.stray-end",
            dspan, "#end without matching #if"
        );
    }

    _el_pp_pop_if_frame(pp);
    return true;
}

bool _el_pp_skip_if(ElPreproc* pp) {
    if (!_el_pp_skip_expr(pp)) {
        return false;
    }
    pp->skip_depth++;
    return true;
}

bool _el_pp_skip_else(ElPreproc* pp, ElSourceSpan dspan) {
    if (pp->skip_depth > 1) {
        return true;
    }

    if (!check_else(pp, dspan)) {
        return false;
    }

    if (pp->if_stack->branch_taken) {
        return true;
    }

    _el_pp_enter_if_branch(pp);
    pp->skip_depth = 0;
    return true;
}

bool _el_pp_skip_elif(ElPreproc* pp, ElSourceSpan dspan) {
    if (pp->skip_depth > 1) {
        if (!_el_pp_skip_expr(pp)) {
            return false;
        }
        return true;
    }

    if (pp->if_stack == NULL) {
        return el_diag_report(
            pp->diag, EL_DIAG_ERROR, "pp.stray-elif",
            dspan, "#elif without matching #if"
        );
    }

    if (pp->if_stack->had_else) {
        return el_diag_report(
            pp->diag, EL_DIAG_ERROR, "pp.elif-after-else",
            dspan, "#elif after #else"
        );
    }

    if (pp->if_stack->branch_taken) {
        if (!_el_pp_skip_expr(pp)) {
            return false;
        }
        return true;
    }

    bool cond;
    if (!eval_cond(pp, dspan, &cond)) {
        return false;
    }

    if (cond) {
        _el_pp_enter_if_branch(pp);
        pp->skip_depth = 0;
    }
    return true;
}

bool _el_pp_skip_end(ElPreproc* pp) {
    if (pp->skip_depth > 1) {
        pp->skip_depth--;
        return true;
    }

    pp->skip_depth = 0;
    _el_pp_pop_if_frame(pp);
    return true;
}
