#include "binder-internals.h"

#include <elash/diag/engine.h>
#include <elash/util/assert.h>
#include <elash/util/int128.h>
#include <elash/util/todo.h>

#include <elash/hir/type/ref.h>
#include <elash/hir/tree/expr.h>
#include <elash/hir/tree/expr/intr.h>

// to reduce boilerplate.
#define type_eql el_hir_type_eql

static inline bool is_fixed_width(ElHirIntWidth width) {
    return width != EL_HIR_IWIDTH_NATIVE && width != EL_HIR_IWIDTH_EFFICIENT;
}
static inline bool is_fixed_fp_width(ElHirFpWidth width) {
    return width != EL_HIR_FPWIDTH_EFFICIENT;
}

static inline bool is_distinct_conv(ElHirType* from, ElHirType* to) {
    return (from->kind == EL_HIR_TYPE_DISTINCT && el_hir_type_eql(from->as.distinct.orig, to)) ||
            (to->kind == EL_HIR_TYPE_DISTINCT && el_hir_type_eql(to->as.distinct.orig, from));
}

ElHirExpr* _cast_untyped(ElBinder* binder, ElSourceSpan span, ElHirExpr* expr, ElHirType* to);

// NOLINTNEXTLINE(readability-function-cognitive-complexity): it's all good
ElHirExpr* _el_binder_eval_const_cast(ElBinder* binder, ElSourceSpan span, ElHirExpr* expr, ElHirType* to_orig) {
    if (expr == NULL || to_orig == NULL) return NULL;
    if (expr->kind == EL_HIR_EXPR_LITERAL)
        return _cast_untyped(binder, span, expr, to_orig);

    EL_ASSERT(expr->kind == EL_HIR_EXPR_CONST, "eval const cast requires a constant operand");

    ElHirType* to = to_orig;
    if (to->kind == EL_HIR_TYPE_DISTINCT)
        to = el_hir_type_unwrap_distinct(to);

    ElHirType* from = expr->type;
    if (from == NULL) return NULL;
    if (from->kind == EL_HIR_TYPE_DISTINCT)
        from = el_hir_type_unwrap_distinct(from);

    if (to->kind != EL_HIR_TYPE_PRIM || from->kind != EL_HIR_TYPE_PRIM)
        return NULL;

    switch (to->as.prim.kind) {
    case EL_PRIMTYPE_INT:
        switch (from->as.prim.kind) {
        case EL_PRIMTYPE_INT: {
            ElInt128 wrapped = _el_binder_wrap_typed_int(binder, span, to, expr->as.constant.as.int_);
            return el_hir_new_int_constant(binder->arena, span, to, wrapped);
        }
        case EL_PRIMTYPE_FLOAT:
            return el_hir_new_int_constant(
                binder->arena, span, to,
                _el_binder_wrap_typed_int(binder, span, to, EL_INT128((int64_t)expr->as.constant.as.float_))
            );
        case EL_PRIMTYPE_BOOL:
        case EL_PRIMTYPE_VOID:
            EL_UNREACHABLE("invalid cast");
        }
        EL_UNREACHABLE_ENUM_VAL(ElHirPrimTypeKind, from->as.prim.kind);
    case EL_PRIMTYPE_BOOL:
        switch (from->as.prim.kind) {
        case EL_PRIMTYPE_BOOL:
            return el_hir_new_bool_constant(binder->arena, span, to, expr->as.constant.as.bool_);
        case EL_PRIMTYPE_FLOAT:
        case EL_PRIMTYPE_INT:
        case EL_PRIMTYPE_VOID:
            EL_UNREACHABLE("invalid cast");
        }
        EL_UNREACHABLE_ENUM_VAL(ElHirPrimTypeKind, from->as.prim.kind);
    case EL_PRIMTYPE_FLOAT:
        switch (from->as.prim.kind) {
        case EL_PRIMTYPE_INT:
            return el_hir_new_float_constant(binder->arena, span, to, (double)el_i128_lo(expr->as.constant.as.int_));
        case EL_PRIMTYPE_FLOAT:
            return el_hir_new_float_constant(binder->arena, span, to, expr->as.constant.as.float_);
        case EL_PRIMTYPE_BOOL:
        case EL_PRIMTYPE_VOID:
            EL_UNREACHABLE("invalid cast");
        }
        EL_UNREACHABLE_ENUM_VAL(ElHirPrimTypeKind, from->as.prim.kind);
    case EL_PRIMTYPE_VOID:
        EL_UNREACHABLE("invalid cast");
    }
    EL_UNREACHABLE_ENUM_VAL(ElHirPrimTypeKind, to->as.prim.kind);
}

ElHirExpr* _el_binder_explicit_cast(ElBinder* binder, ElSourceSpan span, ElHirExpr* expr, ElHirType* to) {
    EL_ASSERT(expr != NULL, "shouldn't be null here");

    ElHirType* from = expr->type;
    if (from == NULL)
        return _cast_untyped(binder, span, expr, to);

    if (type_eql(from, to)) return expr;

    if (to->kind == EL_HIR_TYPE_DISTINCT) {
        if (!_el_binder_ensure_complete(binder, span, to))
            return NULL;
        ElHirExpr* casted = _el_binder_implicit_cast(binder, span, expr, to->as.distinct.orig);
        if (casted != NULL) return el_hir_new_semcast_expr(binder->arena, expr->span, to, casted);
    }
    if (from->kind == EL_HIR_TYPE_DISTINCT) {
        if (!_el_binder_ensure_complete(binder, span, from))
            return NULL;
        expr = el_hir_new_semcast_expr(binder->arena, expr->span, from->as.distinct.orig, expr);
        from = expr->type;
    }

    if (from->kind == EL_HIR_TYPE_PRIM && to->kind == EL_HIR_TYPE_PRIM) {
        bool is_int_conv = from->as.prim.kind == EL_PRIMTYPE_INT && to->as.prim.kind == EL_PRIMTYPE_INT;
        bool is_float_conv = (from->as.prim.kind == EL_PRIMTYPE_FLOAT || from->as.prim.kind == EL_PRIMTYPE_INT)
                            && (to->as.prim.kind == EL_PRIMTYPE_FLOAT || to->as.prim.kind == EL_PRIMTYPE_INT);
        if (is_int_conv || is_float_conv) {
            return el_hir_new_semcast_expr(binder->arena, expr->span, to, expr);
        }
    }

    return _el_binder_implicit_cast(binder, span, expr, to);
}

static ElHirExpr* implicit_cast_array(ElBinder* binder, ElSourceSpan span, ElHirExpr* expr, ElHirType* to, bool* bad) {
    ElHirType* from = expr->type;

    *bad = false;
    if (to->kind == EL_HIR_TYPE_SLICE) {
        return el_hir_new_make_slice_intr(
            binder->arena,
            expr->span,
            _el_binder_implicit_cast(binder, span, expr, el_hir_new_raw_slice_type(binder->arena, from->as.array.base)),
            el_hir_new_int_constant(binder->arena, EL_SRCSPAN_NULL, binder->builtins->type_usize, EL_INT128((int64_t)from->as.array.size))
        );
    } else if (to->kind == EL_HIR_TYPE_RWSLICE) {
        if (type_eql(to->as.rwslice.base, from->as.array.base)) {
            // &(expr)[0] as T[&]
            ElHirType* base_type = from->as.array.base;
            return el_hir_new_bitcast_expr(binder->arena, expr->span, to,
                el_hir_new_unary_expr(
                    binder->arena, expr->span,
                    el_hir_new_ref_type(binder->arena, base_type),
                    EL_SEMA_UNARY_OP_ADDROF,
                    el_hir_new_bin_expr(binder->arena, EL_SRCSPAN_NULL, base_type, EL_SEMA_BIN_OP_INDEX,
                        expr, el_hir_new_int_constant(binder->arena, EL_SRCSPAN_NULL, binder->builtins->type_int, EL_INT128(0))
            )));
        }
    } else if (to->kind == EL_HIR_TYPE_REF) {
        // let's give the user some nice error message in this case
        el_diag_report(
            binder->diag, EL_DIAG_ERROR, "sema.invalid-cast", span,
            "invalid cast from array type ${from} to ${to} pointer",
            EL_DIAG_TYPE("from", from), EL_DIAG_TYPE("to", to),
        );
        el_diag_help(
            binder->diag, "did you meant to use a raw slice ('${type}')?",
            EL_DIAG_TYPE("type", el_hir_new_raw_slice_type(binder->arena, to->as.ref.base)),
        );

        *bad = true;
        return NULL;
    }

    return NULL;
}

static ElHirExpr* implicit_cast_slice(ElBinder* binder, ElHirExpr* expr, ElHirType* to) {
    if (to->kind == EL_HIR_TYPE_RWSLICE) {
        if (type_eql(expr->type->as.slice.base, to->as.rwslice.base)) {
            return el_hir_new_slice_data_intr(
                binder->arena, expr->span,
                to, expr
            );
        }
    }

    return NULL;
}

static ElHirExpr* implicit_cast_prim(ElBinder* binder, ElHirExpr* expr, ElHirType* to) {
    ElHirType* from = expr->type;

    if (from->as.prim.kind == EL_PRIMTYPE_INT && to->as.prim.kind == EL_PRIMTYPE_INT) {
        // the type of these expressions is an anonymous union
        // and using auto/typeof requires C23 which is not widely
        // supported so let's stick to #define
        #define from_itype (&from->as.prim.as.integral)
        #define to_itype   (&to->as.prim.as.integral)
        bool is_valid = from_itype->is_signed == to_itype->is_signed
                    && (from_itype->width     == to_itype->width
                    || (is_fixed_width(from_itype->width) && is_fixed_width(to_itype->width)
                    &&  from_itype->width     <= to_itype->width));
        if (is_valid) return el_hir_new_semcast_expr(binder->arena, expr->span, to, expr);
    } else if (from->as.prim.kind == EL_PRIMTYPE_FLOAT && to->as.prim.kind == EL_PRIMTYPE_FLOAT) {
        // same reason as before, don't blame me plz
        #define from_fptype (&from->as.prim.as.fp)
        #define to_fptype   (&to->as.prim.as.fp)
        bool is_valid = from_fptype->width == to_fptype->width
                    || (is_fixed_fp_width(from_fptype->width) && is_fixed_fp_width(to_fptype->width)
                    &&  from_fptype->width < to_fptype->width);
        if (is_valid) return el_hir_new_semcast_expr(binder->arena, expr->span, to, expr);
    }

    return NULL;
}

ElHirExpr* _el_binder_implicit_cast(ElBinder* binder, ElSourceSpan span, ElHirExpr* expr, ElHirType* to) {
    ElHirType* from = expr->type;
    if (from == NULL) {
        if (to->kind == EL_HIR_TYPE_OPT) {
            if (expr->kind == EL_HIR_EXPR_LITERAL && expr->as.literal.kind == EL_HIR_LITERAL_NULL) {
                return el_hir_new_null_opt_intr(binder->arena, span, to);
            }
            ElHirExpr* base = _cast_untyped(binder, span, expr, to->as.opt.base);
            if (base == NULL) return NULL;
            return el_hir_new_some_opt_intr(binder->arena, span, to, base);
        }
        return _cast_untyped(binder, span, expr, to);
    }

    if (type_eql(from, to)) return expr;

    if (to->kind == EL_HIR_TYPE_OPT) {
        ElHirExpr* casted = _el_binder_implicit_cast(binder, span, expr, to->as.opt.base);
        if (casted != NULL) {
            return el_hir_new_some_opt_intr(binder->arena, span, to, casted);
        }
    }

    if (from->kind == EL_HIR_TYPE_ARRAY) {
        bool bad;
        ElHirExpr* result = implicit_cast_array(binder, span, expr, to, &bad);
        if (result != NULL) {
            return result;
        } else {
            if (bad) return NULL;
        }
    }

    if (from->kind == EL_HIR_TYPE_SLICE) {
        ElHirExpr* result = implicit_cast_slice(binder, expr, to);
        if (result != NULL)
            return result;
    }

    if (from->kind == EL_HIR_TYPE_PRIM && to->kind == EL_HIR_TYPE_PRIM) {
        ElHirExpr* result = implicit_cast_prim(binder, expr, to);
        if (result != NULL)
            return result;
    }

    if (is_distinct_conv(from, to)) {
        el_diag_report(
            binder->diag, EL_DIAG_ERROR, "sema.invalid-cast", span,
            "cannot implicitly convert from ${from} to ${to}",
            EL_DIAG_TYPE("from", from), EL_DIAG_TYPE("to", to),
        );
        el_diag_help(
            binder->diag, "distinct types require an explicit cast with 'as'",
        );
        return NULL;
    }

    el_diag_report(
        binder->diag, EL_DIAG_ERROR, "sema.invalid-cast", span,
        "invalid cast from '${from}' to '${to}'",
        EL_DIAG_TYPE("from", from), EL_DIAG_TYPE("to", to),
    );
    return NULL;
}

static ElHirExpr* cast_untyped_compound(ElBinder* binder, ElSourceSpan span, ElHirExpr* expr, ElHirType* to) {
    switch (expr->kind) {
    case EL_HIR_EXPR_BINARY: {
        ElHirBinExpr* bin = &expr->as.binary;
        ElHirExpr* left = _el_binder_implicit_cast(binder, bin->left->span, bin->left, to);
        ElHirExpr* right = _el_binder_implicit_cast(binder, bin->right->span, bin->right, to);
        if (left == NULL || right == NULL) return NULL;

        ElHirType* result_ty = el_bin_op_is_comparison(bin->op)
            ? binder->builtins->type_bool
            : to;
        ElHirExpr* out = el_hir_new_bin_expr(binder->arena, expr->span, result_ty, bin->op, left, right);
        out = _el_binder_simplify_expr(binder, out);
        if (out != NULL && out->type != NULL && !type_eql(out->type, to))
            return _el_binder_implicit_cast(binder, span, out, to);
        return out;
    }
    case EL_HIR_EXPR_UNARY: {
        ElUnaryOp op = expr->as.unary.op;
        if (op == EL_SEMA_UNARY_OP_PRE_INC || op == EL_SEMA_UNARY_OP_PRE_DEC
            || op == EL_SEMA_UNARY_OP_POST_INC || op == EL_SEMA_UNARY_OP_POST_DEC
            || op == EL_SEMA_UNARY_OP_ADDROF || op == EL_SEMA_UNARY_OP_DEREF
            || op == EL_SEMA_UNARY_OP_OPT_UNWRAP) {
            return el_diag_report(
                binder->diag, EL_DIAG_ERROR, "sema.invalid-cast", span,
                "operator cannot be used in this context"
            );
        }

        ElHirExpr* operand = _el_binder_implicit_cast(
            binder, expr->as.unary.operand->span, expr->as.unary.operand, to
        );
        if (operand == NULL) return NULL;

        ElHirExpr* out = el_hir_new_unary_expr(binder->arena, expr->span, to, op, operand);
        return _el_binder_simplify_expr(binder, out);
    }
    default:
        return el_diag_report(
            binder->diag, EL_DIAG_ERROR, "sema.invalid-cast",
            span, "untyped expression cannot be converted to type ${to}",
            EL_DIAG_TYPE("to", to),
        );
    }
}

// TODO: split this function into smaller helpers
ElHirExpr* _cast_untyped(ElBinder* binder, ElSourceSpan span, ElHirExpr* expr, ElHirType* to) {
    if (to->kind == EL_HIR_TYPE_DISTINCT) {
        if (!_el_binder_ensure_complete(binder, span, to))
            return NULL;
        ElHirExpr* casted = _cast_untyped(binder, span, expr, to->as.distinct.orig);
        if (casted == NULL) return NULL;
        return el_hir_new_semcast_expr(binder->arena, expr->span, to, casted);
    }

    if (expr->kind != EL_HIR_EXPR_LITERAL)
        return cast_untyped_compound(binder, span, expr, to);

    ElHirLiteral* lit = &expr->as.literal;

    if (to->kind == EL_HIR_TYPE_PRIM) {
        ElHirPrimType* prim = &to->as.prim;
        switch (lit->kind) {
        case EL_HIR_LITERAL_INT:
            if (prim->kind == EL_PRIMTYPE_INT) {
                ElInt128 wrapped = _el_binder_wrap_typed_int(binder, expr->span, to, lit->of.int_);
                return el_hir_new_int_constant(binder->arena, expr->span, to, wrapped);
            } else if (prim->kind == EL_PRIMTYPE_FLOAT) {
                return el_hir_new_float_constant(binder->arena, expr->span, to, (double)el_i128_lo(lit->of.int_));
            }
            break;
        case EL_HIR_LITERAL_CHAR:
            if (prim->kind == EL_PRIMTYPE_INT) {
                ElInt128 wrapped = _el_binder_wrap_typed_int(binder, expr->span, to, EL_INT128((int64_t)lit->of.char_));
                return el_hir_new_int_constant(binder->arena, expr->span, to, wrapped);
            }
            break;
        case EL_HIR_LITERAL_BOOL:
            if (prim->kind == EL_PRIMTYPE_BOOL) {
                return el_hir_new_bool_constant(binder->arena, expr->span, to, lit->of.bool_);
            }
            break;
        case EL_HIR_LITERAL_FLOAT:
            if (prim->kind == EL_PRIMTYPE_FLOAT) {
                return el_hir_new_float_constant(binder->arena, expr->span, to, lit->of.float_);
            } else if (prim->kind == EL_PRIMTYPE_INT) {
                ElInt128 wrapped = _el_binder_wrap_typed_int(binder, expr->span, to, EL_INT128((int64_t)lit->of.float_));
                return el_hir_new_int_constant(binder->arena, expr->span, to, wrapped);
            }
            break;
        default:
            break; // handled below
        }
    }

    if (lit->kind == EL_HIR_LITERAL_STRING) {
        if (to->kind == EL_HIR_TYPE_ARRAY) {
            if (el_hir_type_eql(to->as.array.base, binder->builtins->type_char) && to->as.array.size == lit->of.str_.len) {
                return el_hir_new_string_const(binder->arena, expr->span, to, lit->of.str_, EL_STORAGECLS_STATIC);
            }
        } else if (to->kind == EL_HIR_TYPE_SLICE && el_hir_type_eql(to->as.slice.base, binder->builtins->type_char)) {
            return _el_binder_implicit_cast(
                binder, expr->span,
                el_hir_new_string_const(binder->arena, expr->span,
                    el_hir_new_array_type(binder->arena, binder->builtins->type_char, lit->of.str_.len),
                    lit->of.str_, EL_STORAGECLS_STATIC),
                to
            );
        }
    }

    if (lit->kind == EL_HIR_LITERAL_NULL) {
        if (to->kind == EL_HIR_TYPE_OPT) {
            return el_hir_new_null_opt_intr(binder->arena, expr->span, to);
        } else if (to->kind == EL_HIR_TYPE_REF) {
            el_diag_report(
                binder->diag, EL_DIAG_ERROR, "sema.invalid-cast",
                span, "untyped ${of} literal cannot be converted to reference type '${to}'",
                EL_DIAG_STRING("of", el_hir_literal_kind_to_string(lit->kind)),
                EL_DIAG_TYPE("to", to),
            );
            el_diag_help(
                binder->diag, "in elash, references are non-nullable by default",
            );
            el_diag_help(
                binder->diag, "use '${type}?' if you need nullability",
                EL_DIAG_TYPE("type", to),
            );
            return NULL;
        }
    }

    return el_diag_report(
        binder->diag, EL_DIAG_ERROR, "sema.invalid-cast",
        span, "untyped ${of} literal cannot be converted to type '${to}'",
        EL_DIAG_STRING("of", el_hir_literal_kind_to_string(lit->kind)),
        EL_DIAG_TYPE("to", to),
    );
}

ElHirExpr* _el_binder_apply_default_type(ElBinder* binder, ElHirExpr* expr) {
    if (expr->type != NULL) return expr;
    if (expr->kind == EL_HIR_EXPR_LITERAL) {
        ElHirLiteral* lit = &expr->as.literal;
        switch (lit->kind) {
        case EL_HIR_LITERAL_STRING: {
            ElHirType* type = el_hir_new_array_type(binder->arena, binder->builtins->type_char, lit->of.str_.len);
            return el_hir_new_string_const(binder->arena, expr->span, type, lit->of.str_, EL_STORAGECLS_STATIC);
        }
        case EL_HIR_LITERAL_INT:
            return el_hir_new_int_constant(binder->arena, expr->span, binder->builtins->type_int, lit->of.int_);
        case EL_HIR_LITERAL_CHAR:
            return el_hir_new_char_constant(binder->arena, expr->span, binder->builtins->type_char, lit->of.char_);
        case EL_HIR_LITERAL_BOOL:
            return el_hir_new_bool_constant(binder->arena, expr->span, binder->builtins->type_bool, lit->of.bool_);
        case EL_HIR_LITERAL_FLOAT:
            return el_hir_new_float_constant(binder->arena, expr->span, binder->builtins->type_float, lit->of.float_);
        case EL_HIR_LITERAL_NULL:
            return el_diag_report(
                binder->diag, EL_DIAG_ERROR, "sema.invalid-cast",
                expr->span, "untyped null literal requires a target type"
            );
        }
        EL_UNREACHABLE_ENUM_VAL(ElHirLiteralKind, lit->kind);
    }
    return expr;
}
