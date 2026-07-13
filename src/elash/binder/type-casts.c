#include <elash/binder/binder.h>
#include <elash/sema/type/ptr.h>
#include <elash/hir/tree/expr.h>

#include <elash/util/todo.h>

// to reduce boilerplate.
#define type_eql el_sema_type_eql

static inline bool is_fixed_width(ElBasicIntWidth width) {
    return width != EL_INT_WIDTH_NATIVE && width != EL_INT_WIDTH_EFFICIENT;
}

ElHirExpr* _el_binder_explicit_cast(ElBinder* binder, ElSourceSpan span, ElHirExpr* expr, ElType* to) {
    ElType* from = expr->type;
    if (type_eql(from, to)) return expr;

    if (from->kind == EL_TYPE_PRIM && to->kind == EL_TYPE_PRIM) {
        bool is_int_conv = from->as.prim.kind == EL_PRIMTYPE_INT && to->as.prim.kind == EL_PRIMTYPE_INT;
        bool is_char_int_conv = (from->as.prim.kind == EL_PRIMTYPE_INT || from->as.prim.kind == EL_PRIMTYPE_CHAR)
                                && (to->as.prim.kind == EL_PRIMTYPE_INT || to->as.prim.kind == EL_PRIMTYPE_CHAR);
        if (is_int_conv || is_char_int_conv) {
            return el_hir_new_cast_expr(binder->hir_arena, to, expr);
        }
    }

    return _el_binder_implicit_cast(binder, span, expr, to);
}

ElHirExpr* _el_binder_implicit_cast(ElBinder* binder, ElSourceSpan span, ElHirExpr* expr, ElType* to) {
    ElType* from = expr->type;
    if (type_eql(from, to)) return expr;

    if (from->kind == EL_TYPE_ARRAY) {
        if (to->kind == EL_TYPE_SLICE) {
            // this should probably return something like
            // mkslice(arr as int[*], len(arr))
            EL_TODO("implement array to slice casting");
        } else if (to->kind == EL_TYPE_RAW_SLICE) {
            if (type_eql(to->as.raw_slice.base, from->as.array.base)) {
                // &(expr)[0] as T[*]
                ElType* base_type = from->as.array.base;
                return el_hir_new_cast_expr(binder->hir_arena, to,
                    el_hir_new_unary_expr(
                        binder->hir_arena, el_sema_new_ptr_type(binder->type_arena, base_type),
                        EL_SEMA_UNARY_OP_ADDROF,
                        el_hir_new_bin_expr(binder->hir_arena, base_type, EL_SEMA_BIN_OP_INDEX,
                            expr, el_hir_new_int_literal(binder->hir_arena, binder->builtins->type_int, 0)
                )));
            }
        } else if (to->kind == EL_TYPE_PTR) {
            // let's give the user some nice error message in this case
            el_diag_report(
                binder->diag, EL_DIAG_ERROR, "sema.invalid-cast", span,
                "invalid cast from array type '${from}' to '${to}' pointer",
                EL_DIAG_TYPE("from", from), EL_DIAG_TYPE("to", to),
            );
            el_diag_help(
                binder->diag, "did you meant to use a raw slice ('${type}')?",
                EL_DIAG_TYPE("type", el_sema_new_raw_slice_type(binder->type_arena, to->as.ptr.base)),
            );

            return NULL;
        }
    }

    if (from->kind == EL_TYPE_PRIM && to->kind == EL_TYPE_PRIM) {
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
