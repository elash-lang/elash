#include "preproc-internals.h"

#include <elash/sema/unary-op.h>
#include <elash/sema/bin-op.h>

#include <math.h>

bool _el_pp_to_int(ElPpValue* val, int64_t* out) {
    ElPpNum num;
    if (!_el_pp_to_num(val, &num) || num.kind != EL_PP_NUM_INT) {
        return false;
    }
    *out = num.as.int_;
    return true;
}

bool _el_pp_to_num(ElPpValue* val, ElPpNum* out) {
    switch (val->type) {
    case EL_PP_TYPE_INT:
        out->kind = EL_PP_NUM_INT;
        out->as.int_ = val->as.int_;
        return true;
    case EL_PP_TYPE_FLOAT:
        out->kind = EL_PP_NUM_FLOAT;
        out->as.float_ = val->as.float_;
        return true;
    default:
        return false;
    }
}

ElPpValue* _el_pp_apply_numeric_bin(
    ElPreproc* pp, ElSourceSpan span, ElSemaBinOp op, ElPpNum lhs, ElPpNum rhs
) {
    bool fp = lhs.kind == EL_PP_NUM_FLOAT || rhs.kind == EL_PP_NUM_FLOAT;
    double lf = lhs.kind == EL_PP_NUM_FLOAT ? lhs.as.float_ : (double)lhs.as.int_;
    double rf = rhs.kind == EL_PP_NUM_FLOAT ? rhs.as.float_ : (double)rhs.as.int_;
    int64_t li = lhs.kind == EL_PP_NUM_INT ? lhs.as.int_ : (int64_t)lhs.as.float_;
    int64_t ri = rhs.kind == EL_PP_NUM_INT ? rhs.as.int_ : (int64_t)rhs.as.float_;

    switch (op) {
    case EL_SEMA_BIN_OP_ADD:
        return fp ? _el_pp_new_float(pp->iarena, lf + rf) : _el_pp_new_int(pp->iarena, li + ri);
    case EL_SEMA_BIN_OP_SUB:
        return fp ? _el_pp_new_float(pp->iarena, lf - rf) : _el_pp_new_int(pp->iarena, li - ri);
    case EL_SEMA_BIN_OP_MUL:
        return fp ? _el_pp_new_float(pp->iarena, lf * rf) : _el_pp_new_int(pp->iarena, li * ri);

    case EL_SEMA_BIN_OP_DIV:
        if ((!fp && ri == 0) || (fp && rf == 0.0))
            return el_diag_report(
                pp->diag, EL_DIAG_ERROR, "pp.div-by-zero", span, "division by zero"
            );
        return fp ? _el_pp_new_float(pp->iarena, lf / rf) : _el_pp_new_int(pp->iarena, li / ri);
    case EL_SEMA_BIN_OP_MOD:
        if ((!fp && ri == 0) || (fp && rf == 0.0))
            return el_diag_report(
                pp->diag, EL_DIAG_ERROR, "pp.mod-by-zero", span, "modulo by zero"
            );

        if (fp) {
            return _el_pp_new_float(pp->iarena, fmod(lf, rf));
        } else {
            return _el_pp_new_int(pp->iarena, li % ri);
        }

    case EL_SEMA_BIN_OP_BW_AND:
        if (fp) return _el_pp_report_float_bw(pp, span, op);
        return _el_pp_new_int(pp->iarena, li & ri);
    case EL_SEMA_BIN_OP_BW_OR:
        if (fp) return _el_pp_report_float_bw(pp, span, op);
        return _el_pp_new_int(pp->iarena, li | ri);
    case EL_SEMA_BIN_OP_BW_XOR:
        if (fp) return _el_pp_report_float_bw(pp, span, op);
        return _el_pp_new_int(pp->iarena, li ^ ri);
    case EL_SEMA_BIN_OP_BW_IMP:
        if (fp) return _el_pp_report_float_bw(pp, span, op);
        return _el_pp_new_int(pp->iarena, (~li) | ri);
    case EL_SEMA_BIN_OP_SHL:
        if (fp) return _el_pp_report_float_bw(pp, span, op);
        return _el_pp_new_int(pp->iarena, li << ri);
    case EL_SEMA_BIN_OP_SHR:
        if (fp) return _el_pp_report_float_bw(pp, span, op);
        return _el_pp_new_int(pp->iarena, li >> ri);

    default:
        return el_diag_report(
            pp->diag, EL_DIAG_ERROR, "pp.unsupported-op", span,
            "operator '${op}' is not supported in preprocessor expressions",
            EL_DIAG_STRING("op", el_sema_bin_op_to_string(op))
        );
    }
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity): it's fine.
ElPpValue* _el_pp_apply_bin_op(
    ElPreproc* pp, ElSourceSpan span, ElSemaBinOp op, ElPpValue* lhs, ElPpValue* rhs
) {
    if (op == EL_SEMA_BIN_OP_ADD) {
        if (lhs->type == EL_PP_TYPE_STR || rhs->type == EL_PP_TYPE_STR) {
            return _el_pp_strcat(pp, lhs, rhs);
        }
        if (lhs->type == EL_PP_TYPE_LIST || rhs->type == EL_PP_TYPE_LIST) {
            return _el_pp_listcat(pp, lhs, rhs);
        }
    }

    if (op == EL_SEMA_BIN_OP_INDEX) {
        int64_t index = 0;
        if (!_el_pp_to_int(rhs, &index)) {
            return el_diag_report(
                pp->diag, EL_DIAG_ERROR, "pp.invalid-op", span,
                "ndex must be an integer, got ${type}",
                EL_DIAG_STRING("type", _el_pp_type_name(rhs->type))
            );
        }

        if (index < 0) {
            return el_diag_report(
                pp->diag, EL_DIAG_ERROR, "pp.out-of-bounds",
                span, "index cannot be negative"
            );
        }

        if (lhs->type == EL_PP_TYPE_LIST) {
            if ((usize)index >= lhs->as.list_.count) {
                return el_diag_report(
                    pp->diag, EL_DIAG_ERROR, "pp.out-of-bounds", span,
                    "list index ${index} out of bounds (length ${len})",
                    EL_DIAG_INT("index", (int)index),
                    EL_DIAG_INT("len", (int)lhs->as.list_.count)
                );
            }
            return lhs->as.list_.values[(usize)index];
        }

        if (lhs->type == EL_PP_TYPE_STR) {
            if ((usize)index >= lhs->as.str_.len) {
                return el_diag_report(
                    pp->diag, EL_DIAG_ERROR, "pp.out-of-bounds", span,
                    "string index ${index} out of bounds (length ${len})",
                    EL_DIAG_INT("index", (int)index),
                    EL_DIAG_INT("len", (int)lhs->as.str_.len)
                );
            }
            return _el_pp_new_char(pp->iarena, lhs->as.str_.data[(usize)index]);
        }

        return el_diag_report(
            pp->diag, EL_DIAG_ERROR, "pp.invalid-op", span,
            "operator '[]' is not defined for type ${type}",
            EL_DIAG_STRING("type", _el_pp_type_name(lhs->type))
        );
    }

    if (op == EL_SEMA_BIN_OP_EQ || op == EL_SEMA_BIN_OP_NEQ ||
        op == EL_SEMA_BIN_OP_LT || op == EL_SEMA_BIN_OP_LTE ||
        op == EL_SEMA_BIN_OP_GT || op == EL_SEMA_BIN_OP_GTE) {
        bool res;
        if (op == EL_SEMA_BIN_OP_EQ || op == EL_SEMA_BIN_OP_NEQ) {
            res = _el_pp_value_eq(pp, lhs, rhs, span);
            if (op == EL_SEMA_BIN_OP_NEQ) res = !res;
        } else if (op == EL_SEMA_BIN_OP_LT || op == EL_SEMA_BIN_OP_GTE) {
            res = _el_pp_value_lt(pp, lhs, rhs, span);
            if (op == EL_SEMA_BIN_OP_GTE) res = !res;
        } else {
            res = _el_pp_value_gt(pp, lhs, rhs, span);
            if (op == EL_SEMA_BIN_OP_LTE) res = !res;
        }

        if (el_diag_engine_has_errors(pp->diag)) {
            return NULL;
        }
        return _el_pp_new_bool(pp->iarena, res);
    }

    if (op == EL_SEMA_BIN_OP_AND) {
        if (lhs->type != EL_PP_TYPE_BOOL || rhs->type != EL_PP_TYPE_BOOL)
            return _el_pp_report_non_bool_logical(pp, span, op);
        return _el_pp_new_bool(pp->iarena, lhs->as.bool_ && rhs->as.bool_);
    }
    if (op == EL_SEMA_BIN_OP_OR) {
        if (lhs->type != EL_PP_TYPE_BOOL || rhs->type != EL_PP_TYPE_BOOL)
            return _el_pp_report_non_bool_logical(pp, span, op);
        return _el_pp_new_bool(pp->iarena, lhs->as.bool_ || rhs->as.bool_);
    }
    if (op == EL_SEMA_BIN_OP_IMP) {
        if (lhs->type != EL_PP_TYPE_BOOL || rhs->type != EL_PP_TYPE_BOOL)
            return _el_pp_report_non_bool_logical(pp, span, op);
        return _el_pp_new_bool(pp->iarena, !lhs->as.bool_ || rhs->as.bool_);
    }

    ElPpNum lnum;
    ElPpNum rnum;
    if (!_el_pp_to_num(lhs, &lnum) || !_el_pp_to_num(rhs, &rnum)) {
        return el_diag_report(
            pp->diag, EL_DIAG_ERROR, "pp.invalid-op", span,
            "invalid operands to '${op}': ${left} and ${right}",
            EL_DIAG_STRING("op", el_sema_bin_op_to_string(op)),
            EL_DIAG_STRING("left", _el_pp_type_name(lhs->type)),
            EL_DIAG_STRING("right", _el_pp_type_name(rhs->type))
        );
    }

    return _el_pp_apply_numeric_bin(pp, span, op, lnum, rnum);
}

ElPpValue* _el_pp_apply_unary_op(ElPreproc* pp, ElSourceSpan span, ElSemaUnaryOp op, ElPpValue* operand) {
    if (op == EL_SEMA_UNARY_OP_NOT) {
        if (operand->type != EL_PP_TYPE_BOOL)
            return _el_pp_report_non_bool_logical_unary(pp, span, op);
        return _el_pp_new_bool(pp->iarena, !operand->as.bool_);
    }

    ElPpNum num;
    if (!_el_pp_to_num(operand, &num)) {
        return el_diag_report(
            pp->diag, EL_DIAG_ERROR, "pp.invalid-op", span,
            "invalid operand to '${op}': ${type}",
            EL_DIAG_STRING("op", el_sema_unary_op_to_string(op)),
            EL_DIAG_STRING("type", _el_pp_type_name(operand->type))
        );
    }

    if (num.kind == EL_PP_NUM_FLOAT) {
        switch (op) {
        case EL_SEMA_UNARY_OP_POS:
            return _el_pp_new_float(pp->iarena, +num.as.float_);
        case EL_SEMA_UNARY_OP_NEG:
            return _el_pp_new_float(pp->iarena, -num.as.float_);
        default:
            return el_diag_report(
                pp->diag, EL_DIAG_ERROR, "pp.invalid-op", span,
                "operator '${op}' is not defined for floating-point operands",
                EL_DIAG_STRING("op", el_sema_unary_op_to_string(op))
            );
        }
    }

    int64_t val = num.as.int_;
    switch (op) {
    case EL_SEMA_UNARY_OP_POS:
        return _el_pp_new_int(pp->iarena, +val);
    case EL_SEMA_UNARY_OP_NEG:
        return _el_pp_new_int(pp->iarena, -val);
    case EL_SEMA_UNARY_OP_BW_NOT:
        return _el_pp_new_int(pp->iarena, ~val);
    case EL_SEMA_UNARY_OP_PRE_INC:
    case EL_SEMA_UNARY_OP_PRE_DEC:
    case EL_SEMA_UNARY_OP_POST_INC:
    case EL_SEMA_UNARY_OP_POST_DEC:
        return _el_pp_report_incdec(pp, span);
    default:
        return el_diag_report(
            pp->diag, EL_DIAG_ERROR, "pp.invalid-op", span,
            "unsupported unary operator '${op}' in preprocessor expressions",
            EL_DIAG_STRING("op", el_sema_unary_op_to_string(op))
        );
    }
}
