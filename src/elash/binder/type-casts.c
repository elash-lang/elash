#include <elash/binder/binder.h>
#include <elash/hir/type/ref.h>
#include <elash/hir/tree/expr.h>

#include <elash/util/todo.h>
#include <elash/diag/meta.h>

// to reduce boilerplate.
#define type_eql el_hir_type_eql

static inline bool is_fixed_width(ElHirIntWidth width) {
    return width != EL_HIR_IWIDTH_NATIVE && width != EL_HIR_IWIDTH_EFFICIENT;
}

ElHirExpr* _cast_untyped(ElBinder* binder, ElSourceSpan span, ElHirExpr* expr, ElHirType* to);

ElHirExpr* _el_binder_explicit_cast(ElBinder* binder, ElSourceSpan span, ElHirExpr* expr, ElHirType* to) {
    ElHirType* from = expr->type;
    if (from == NULL)
        return _cast_untyped(binder, span, expr, to);

    if (type_eql(from, to)) return expr;

    if (from->kind == EL_HIR_TYPE_PRIM && to->kind == EL_HIR_TYPE_PRIM) {
        bool is_int_conv = from->as.prim.kind == EL_PRIMTYPE_INT && to->as.prim.kind == EL_PRIMTYPE_INT;
        bool is_char_int_conv = (from->as.prim.kind == EL_PRIMTYPE_INT || from->as.prim.kind == EL_PRIMTYPE_CHAR)
                                && (to->as.prim.kind == EL_PRIMTYPE_INT || to->as.prim.kind == EL_PRIMTYPE_CHAR);
        if (is_int_conv || is_char_int_conv) {
            return el_hir_new_cast_expr(binder->hir_arena, to, expr);
        }
    }

    return _el_binder_implicit_cast(binder, span, expr, to);
}

ElHirExpr* _el_binder_implicit_cast(ElBinder* binder, ElSourceSpan span, ElHirExpr* expr, ElHirType* to) {
    ElHirType* from = expr->type;
    if (from == NULL)
        return _cast_untyped(binder, span, expr, to);

    if (type_eql(from, to)) return expr;

    if (from->kind == EL_HIR_TYPE_ARRAY) {
        if (to->kind == EL_HIR_TYPE_SLICE) {
            return el_hir_new_make_slice_intr(
                binder->hir_arena,
                _el_binder_implicit_cast(binder, span, expr, el_hir_new_raw_slice_type(binder->type_arena, from->as.array.base)),
                el_hir_new_int_constant(binder->hir_arena, binder->builtins->type_usize, (int64_t)from->as.array.size)
            );
        } else if (to->kind == EL_HIR_TYPE_RWSLICE) {
            if (type_eql(to->as.rwslice.base, from->as.array.base)) {
                // &(expr)[0] as T[*]
                ElHirType* base_type = from->as.array.base;
                return el_hir_new_cast_expr(binder->hir_arena, to,
                    el_hir_new_unary_expr(
                        binder->hir_arena, el_hir_new_ref_type(binder->type_arena, base_type),
                        EL_SEMA_UNARY_OP_ADDROF,
                        el_hir_new_bin_expr(binder->hir_arena, base_type, EL_SEMA_BIN_OP_INDEX,
                            expr, el_hir_new_int_constant(binder->hir_arena, binder->builtins->type_int, 0)
                )));
            }
        } else if (to->kind == EL_HIR_TYPE_REF) {
            // let's give the user some nice error message in this case
            el_diag_report(
                binder->diag, EL_DIAG_ERROR, "sema.invalid-cast", span,
                "invalid cast from array type '${from}' to '${to}' pointer",
                EL_DIAG_TYPE("from", from), EL_DIAG_TYPE("to", to),
            );
            el_diag_help(
                binder->diag, "did you meant to use a raw slice ('${type}')?",
                EL_DIAG_TYPE("type", el_hir_new_raw_slice_type(binder->type_arena, to->as.ref.base)),
            );

            return NULL;
        }
    }

    if (from->kind == EL_HIR_TYPE_PRIM && to->kind == EL_HIR_TYPE_PRIM) {
        if (from->as.prim.kind == EL_PRIMTYPE_INT && to->as.prim.kind == EL_PRIMTYPE_INT) {
            // the type of these expressions is an anonymous union
            // and using auto/typedef requires C23 which is not widely
            // supported so let's stick to #define
            #define from_itype (&from->as.prim.as.integral)
            #define to_itype   (&to->as.prim.as.integral)
            bool is_valid = from_itype->is_signed == to_itype->is_signed
                        && (from_itype->width     == to_itype->width
                        || (is_fixed_width(from_itype->width) && is_fixed_width(to_itype->width)
                        &&  from_itype->width     <= to_itype->width));
            if (is_valid) return el_hir_new_cast_expr(binder->hir_arena, to, expr);
        }
    }

    el_diag_report(
        binder->diag, EL_DIAG_ERROR, "sema.invalid-cast", span,
        "invalid cast from '${from}' to '${to}'",
        EL_DIAG_TYPE("from", from), EL_DIAG_TYPE("to", to),
    );
    return NULL;
}

ElHirExpr* _cast_untyped(ElBinder* binder, ElSourceSpan span, ElHirExpr* expr, ElHirType* to) {
    (void) span;
    if (to->kind == EL_HIR_TYPE_PRIM) {
        switch (expr->as.untyped_lit.kind) {
        case EL_HIR_UNTYPED_INT:
            if (to->as.prim.kind == EL_PRIMTYPE_INT) {
                return el_hir_new_int_constant(binder->hir_arena, to, expr->as.untyped_lit.of.int_);
            } else if (to->as.prim.kind == EL_PRIMTYPE_CHAR) {
                return el_hir_new_char_constant(binder->hir_arena, to, (char)expr->as.untyped_lit.of.int_);
            }
            break;
        case EL_HIR_UNTYPED_CHAR:
            if (to->as.prim.kind == EL_PRIMTYPE_CHAR) {
                return el_hir_new_char_constant(binder->hir_arena, to, expr->as.untyped_lit.of.char_);
            } else if (to->as.prim.kind == EL_PRIMTYPE_INT) {
                return el_hir_new_int_constant(binder->hir_arena, to, (int64_t)expr->as.untyped_lit.of.char_);
            }
            break;
        case EL_HIR_UNTYPED_BOOL:
            if (to->as.prim.kind == EL_PRIMTYPE_BOOL) {
                return el_hir_new_bool_constant(binder->hir_arena, to, expr->as.untyped_lit.of.bool_);
            }
            break;
        }
    }

    return el_diag_report(
        binder->diag, EL_DIAG_ERROR, "invalid-cast",
        span, "untyped ${of} literal cannot be converted to type ${to}",
        EL_DIAG_STRING("of", el_hir_untyped_lit_kind_to_string(expr->as.untyped_lit.kind)),
        EL_DIAG_TYPE("to", to),
    );
}

ElHirExpr* _el_binder_apply_default_type(ElBinder* binder, ElHirExpr* expr) {
    if (expr->type != NULL) return expr;
    if (expr->kind == EL_HIR_EXPR_UNTYPEDLIT) {
        switch (expr->as.untyped_lit.kind) {
        case EL_HIR_UNTYPED_INT:
            return el_hir_new_int_constant(binder->hir_arena, binder->builtins->type_int, expr->as.untyped_lit.of.int_);
        case EL_HIR_UNTYPED_CHAR:
            return el_hir_new_char_constant(binder->hir_arena, binder->builtins->type_char, expr->as.untyped_lit.of.char_);
        case EL_HIR_UNTYPED_BOOL:
            return el_hir_new_bool_constant(binder->hir_arena, binder->builtins->type_bool, expr->as.untyped_lit.of.bool_);
        }
    }
    return expr;
}
