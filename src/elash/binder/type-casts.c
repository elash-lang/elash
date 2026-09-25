#include "binder-internals.h"

#include <elash/diag/engine.h>
#include <elash/util/assert.h>
#include <elash/util/int128.h>
#include <elash/util/todo.h>

#include <elash/hir/type/ref.h>
#include <elash/hir/type/raw-slice.h>
#include <elash/hir/tree/expr.h>
#include <elash/hir/tree/expr/intr.h>

// to reduce boilerplate.
#define type_eql el_hir_type_eql
#define type_eql_unqual el_hir_type_eql_unqual
#define mut_compatible el_hir_type_mut_compatible
#define type_compatible el_hir_type_compatible

#define HANDLE(CASTED, BAD) \
    if (CASTED != NULL) {   \
        return CASTED;      \
    } else if (BAD) {       \
        return NULL;        \
    }

static inline bool is_fixed_width(ElHirIntWidth width) {
    return width != EL_HIR_IWIDTH_NATIVE && width != EL_HIR_IWIDTH_EFFICIENT;
}
static inline bool is_fixed_fp_width(ElHirFpWidth width) {
    return width != EL_HIR_FPWIDTH_EFFICIENT;
}

static inline bool is_distinct_conv(ElHirType* from, ElHirType* to) {
    from = el_hir_type_canonical(from);
    to = el_hir_type_canonical(to);
    return (from->kind == EL_HIR_TYPE_DISTINCT && type_eql(from->as.distinct.orig, to)) ||
            (to->kind == EL_HIR_TYPE_DISTINCT && type_eql(to->as.distinct.orig, from));
}

// types that point to some data
static bool is_view_type(const ElHirType* type) {
    type = el_hir_type_canonical((ElHirType*)type);
    return type != NULL
        && (type->kind == EL_HIR_TYPE_REF
         || type->kind == EL_HIR_TYPE_SLICE
         || type->kind == EL_HIR_TYPE_RWSLICE);
}

static ElHirExpr* cast_quals_only(ElBinder* binder, ElHirExpr* expr, ElHirType* from, ElHirType* to) {
    if (is_view_type(from) || is_view_type(to)) {
        if (!type_compatible(from, to))
            return NULL;
    } else if (!type_eql_unqual(from, to)) {
        return NULL;
    }

    // const T[10] -> T[10] is ok because it copies the underlying data anyway
    return el_hir_new_semcast_expr(binder->arena, expr->span, to, expr);
}

static ElHirType* string_lit_type(ElBinder* binder, usize len) {
    ElHirType* elem = el_hir_type_qualify(binder->arena, binder->builtins->type_char, EL_MUTSPEC_CONST);
    return el_hir_new_array_type(binder->arena, elem, len);
}

ElHirExpr* _cast_untyped(ElBinder* binder, ElSourceSpan span, ElHirExpr* expr, ElHirType* to);

// NOLINTNEXTLINE(readability-function-cognitive-complexity): it's all good
ElHirExpr* _el_binder_eval_const_cast(ElBinder* binder, ElSourceSpan span, ElHirExpr* expr, ElHirType* to_orig) {
    if (expr == NULL || to_orig == NULL) return NULL;
    if (expr->kind == EL_HIR_EXPR_LITERAL)
        return _cast_untyped(binder, span, expr, to_orig);

    EL_ASSERT(expr->kind == EL_HIR_EXPR_CONST, "eval const cast requires a constant operand");

    ElHirType* to_shape = el_hir_type_unwrap(to_orig);
    ElHirType* from = expr->type;
    if (from == NULL) return NULL;
    ElHirType* from_shape = el_hir_type_unwrap(from);

    if (to_shape->kind != EL_HIR_TYPE_PRIM || from_shape->kind != EL_HIR_TYPE_PRIM)
        return NULL;

    switch (to_shape->as.prim.kind) {
    case EL_HIR_PRIMTYPE_INT:
        switch (from_shape->as.prim.kind) {
        case EL_HIR_PRIMTYPE_INT: {
            ElInt128 wrapped = _el_binder_wrap_typed_int(binder, span, to_shape, expr->as.constant.as.int_);
            return el_hir_new_int_constant(binder->arena, span, to_orig, wrapped);
        }
        case EL_HIR_PRIMTYPE_FLOAT:
            return el_hir_new_int_constant(
                binder->arena, span, to_orig,
                _el_binder_wrap_typed_int(binder, span, to_shape, EL_INT128((int64_t)expr->as.constant.as.float_))
            );
        case EL_HIR_PRIMTYPE_BOOL:
        case EL_HIR_PRIMTYPE_VOID:
            EL_UNREACHABLE("invalid cast");
        }
        EL_UNREACHABLE_ENUM_VAL(ElHirPrimTypeKind, from_shape->as.prim.kind);
    case EL_HIR_PRIMTYPE_BOOL:
        switch (from_shape->as.prim.kind) {
        case EL_HIR_PRIMTYPE_BOOL:
            return el_hir_new_bool_constant(binder->arena, span, to_orig, expr->as.constant.as.bool_);
        case EL_HIR_PRIMTYPE_FLOAT:
        case EL_HIR_PRIMTYPE_INT:
        case EL_HIR_PRIMTYPE_VOID:
            EL_UNREACHABLE("invalid cast");
        }
        EL_UNREACHABLE_ENUM_VAL(ElHirPrimTypeKind, from_shape->as.prim.kind);
    case EL_HIR_PRIMTYPE_FLOAT:
        switch (from_shape->as.prim.kind) {
        case EL_HIR_PRIMTYPE_INT:
            return el_hir_new_float_constant(binder->arena, span, to_orig, (double)el_i128_lo(expr->as.constant.as.int_));
        case EL_HIR_PRIMTYPE_FLOAT:
            return el_hir_new_float_constant(binder->arena, span, to_orig, expr->as.constant.as.float_);
        case EL_HIR_PRIMTYPE_BOOL:
        case EL_HIR_PRIMTYPE_VOID:
            EL_UNREACHABLE("invalid cast");
        }
        EL_UNREACHABLE_ENUM_VAL(ElHirPrimTypeKind, from_shape->as.prim.kind);
    case EL_HIR_PRIMTYPE_VOID:
        EL_UNREACHABLE("invalid cast");
    }
    EL_UNREACHABLE_ENUM_VAL(ElHirPrimTypeKind, to_shape->as.prim.kind);
}

ElHirExpr* _el_binder_explicit_cast(ElBinder* binder, ElSourceSpan span, ElHirExpr* expr, ElHirType* to) {
    EL_ASSERT(expr != NULL, "shouldn't be null here");

    if (!_el_binder_ensure_readable(binder, span, expr))
        return NULL;

    ElHirType* from = expr->type;
    if (from == NULL)
        return _cast_untyped(binder, span, expr, to);

    if (type_eql(from, to)) return expr;

    ElHirExpr* quals = cast_quals_only(binder, expr, from, to);
    if (quals != NULL) return quals;

    ElHirType* to_c = el_hir_type_canonical(to);
    ElHirType* from_c = el_hir_type_canonical(from);

    if (to_c->kind == EL_HIR_TYPE_DISTINCT) {
        if (!_el_binder_ensure_complete(binder, span, to_c))
            return NULL;

        ElHirExpr* casted = _el_binder_implicit_cast(binder, span, expr, to_c->as.distinct.orig);
        if (casted != NULL) return el_hir_new_semcast_expr(binder->arena, expr->span, to, casted);
    }

    if (from_c->kind == EL_HIR_TYPE_DISTINCT) {
        if (!_el_binder_ensure_complete(binder, span, from_c))
            return NULL;
        expr = el_hir_new_semcast_expr(binder->arena, expr->span, from_c->as.distinct.orig, expr);
        from = expr->type;
        from_c = el_hir_type_canonical(from);
    }

    if (from_c->kind == EL_HIR_TYPE_PRIM && to_c->kind == EL_HIR_TYPE_PRIM) {
        bool is_int_conv = from_c->as.prim.kind == EL_HIR_PRIMTYPE_INT && to_c->as.prim.kind == EL_HIR_PRIMTYPE_INT;
        bool is_float_conv = (from_c->as.prim.kind == EL_HIR_PRIMTYPE_FLOAT || from_c->as.prim.kind == EL_HIR_PRIMTYPE_INT)
                            && (to_c->as.prim.kind == EL_HIR_PRIMTYPE_FLOAT || to_c->as.prim.kind == EL_HIR_PRIMTYPE_INT);
        if (is_int_conv || is_float_conv) {
            return el_hir_new_semcast_expr(binder->arena, expr->span, to, expr);
        }
    }

    return _el_binder_implicit_cast(binder, span, expr, to);
}

static ElHirExpr* implicit_cast_array(
    ElBinder* binder, ElSourceSpan span, ElHirExpr* expr,
    ElHirType* from, ElHirType* to, ElHirType* from_c, ElHirType* to_c,
    bool* bad
) {
    ElHirType* from_elem = from_c->as.array.base;

    if (to_c->kind == EL_HIR_TYPE_SLICE) {
        ElHirType* to_elem = to_c->as.slice.base;
        if (!type_compatible(from_elem, to_elem)) {
            return NULL;
        }

        // intr 'make-slice' (expr as T[&], len(expr))
        return el_hir_new_make_slice_intr(
            binder->arena, expr->span,
            _el_binder_implicit_cast(binder, span, expr,
                el_hir_new_raw_slice_type(binder->arena, to_elem)),
                el_hir_new_int_constant(binder->arena, EL_SRCSPAN_NULL,
                    binder->builtins->type_usize, EL_INT128((int64_t)from_c->as.array.size))
        );
    } else if (to_c->kind == EL_HIR_TYPE_RWSLICE) {
        ElHirType* to_elem = to_c->as.rwslice.base;
        if (!type_compatible(from_elem, to_elem)) {
            return NULL;
        }

        // &(expr)[0] as T[&]
        return el_hir_new_bitcast_expr(binder->arena, expr->span, to,
            el_hir_new_unary_expr(
                binder->arena, expr->span,
                el_hir_new_ref_type(binder->arena, from_elem),
                EL_UNARY_OP_ADDROF,
                el_hir_new_bin_expr(binder->arena, EL_SRCSPAN_NULL, from_elem, EL_BIN_OP_INDEX,
                    expr, el_hir_new_int_constant(binder->arena, EL_SRCSPAN_NULL, binder->builtins->type_int, EL_INT128(0))))
        );
    } else if (to_c->kind == EL_HIR_TYPE_REF) {
        *bad = true;
        el_diag_report(
            binder->diag, EL_DIAG_ERROR, "sema.invalid-cast", span,
            "invalid cast from array type '${from}' to '${to}' pointer",
            EL_DIAG_TYPE("from", from), EL_DIAG_TYPE("to", to),
        );
        el_diag_help(
            binder->diag, "did you mean to use a raw slice ('${type}')?",
            EL_DIAG_TYPE("type", el_hir_new_raw_slice_type(binder->arena, to_c->as.ref.base)),
        );
        return NULL;
    } else if (to_c->kind == EL_HIR_TYPE_ARRAY) {
        if (from_c->as.array.size == to_c->as.array.size && type_eql_unqual(from_elem, to_c->as.array.base)) {
            return el_hir_new_semcast_expr(binder->arena, expr->span, to, expr);
        }
    }

    return NULL;
}

static ElHirExpr* implicit_cast_slice(ElBinder* binder, ElHirExpr* expr, ElHirType* to, ElHirType* from_c, ElHirType* to_c) {
    if (to_c->kind == EL_HIR_TYPE_RWSLICE && type_compatible(from_c->as.slice.base, to_c->as.rwslice.base)) {
        return el_hir_new_slice_data_intr(
            binder->arena, expr->span,
            to, expr
        );
    }

    return NULL;
}

static ElHirExpr* implicit_cast_prim(ElBinder* binder, ElHirExpr* expr, ElHirType* to, ElHirType* from_c, ElHirType* to_c) {
    if (from_c->as.prim.kind == EL_HIR_PRIMTYPE_INT && to_c->as.prim.kind == EL_HIR_PRIMTYPE_INT) {
        // the type of these expressions is an anonymous union
        // and using auto/typeof requires C23 which is not widely
        // supported so let's stick to_c #define
        #define from_c_itype (&from_c->as.prim.as.integral)
        #define to_itype   (&to_c->as.prim.as.integral)

        bool is_valid = from_c_itype->is_signed == to_itype->is_signed
                    && (from_c_itype->width     == to_itype->width
                    || (is_fixed_width(from_c_itype->width) && is_fixed_width(to_itype->width)
                    &&  from_c_itype->width     <= to_itype->width));

        if (is_valid) {
            return el_hir_new_semcast_expr(binder->arena, expr->span, to, expr);
        }
    } else if (from_c->as.prim.kind == EL_HIR_PRIMTYPE_FLOAT && to_c->as.prim.kind == EL_HIR_PRIMTYPE_FLOAT) {
        // same reason as before, don't blame me plz
        #define from_c_fptype (&from_c->as.prim.as.fp)
        #define to_fptype   (&to_c->as.prim.as.fp)

        bool is_valid = from_c_fptype->width == to_fptype->width
                    || (is_fixed_fp_width(from_c_fptype->width) && is_fixed_fp_width(to_fptype->width)
                    &&  from_c_fptype->width < to_fptype->width);

        if (is_valid) {
            return el_hir_new_semcast_expr(binder->arena, expr->span, to, expr);
        }
    }

    return NULL;
}

static ElHirExpr* implicit_cast_impl(
    ElBinder* binder, ElSourceSpan span, ElHirExpr* expr,
    ElHirType* from, ElHirType* to, ElHirType* from_c, ElHirType* to_c
) {
    if (to_c->kind == EL_HIR_TYPE_OPT) {
        ElHirExpr* casted = _el_binder_implicit_cast(binder, span, expr, to_c->as.opt.base);
        if (casted != NULL) {
            return el_hir_new_some_opt_intr(binder->arena, span, to, casted);
        }
    }

    if (from_c->kind == EL_HIR_TYPE_ARRAY) {
        bool bad = false;
        ElHirExpr* casted = implicit_cast_array(binder, span, expr, from, to, from_c, to_c, &bad);
        HANDLE(casted, bad);
    }

    if (from_c->kind == EL_HIR_TYPE_SLICE) {
        ElHirExpr* casted = implicit_cast_slice(binder, expr, to, from_c, to_c);
        if (casted != NULL) return casted;
    }

    if (from_c->kind == EL_HIR_TYPE_PRIM && to_c->kind == EL_HIR_TYPE_PRIM) {
        ElHirExpr* casted = implicit_cast_prim(binder, expr, to, from_c, to_c);
        if (casted != NULL)
            return casted;
    }

    if (is_distinct_conv(from_c, to_c)) {
        el_diag_report(
            binder->diag, EL_DIAG_ERROR, "sema.invalid-cast", span,
            "cannot implicitly convert from '${from}' to '${to}'",
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

ElHirExpr* _el_binder_implicit_cast(ElBinder* binder, ElSourceSpan span, ElHirExpr* expr, ElHirType* to) {
    // almost everything routes through implicit cast so placing the guard here
    // works and (i hope so) we don't need to put it in any other places
    // the only exception seems to be explicit cast and some binary/unary operators
    if (!_el_binder_ensure_readable(binder, span, expr))
        return NULL;

    ElHirType* from = expr->type;
    if (from == NULL) {
        ElHirType* to_c = el_hir_type_canonical(to);

        if (to_c->kind == EL_HIR_TYPE_OPT) {
            if (expr->kind == EL_HIR_EXPR_LITERAL && expr->as.literal.kind == EL_HIR_LITERAL_NULL) {
                return el_hir_new_null_opt_intr(binder->arena, span, to);
            }
            ElHirExpr* base = _cast_untyped(binder, span, expr, to_c->as.opt.base);
            if (base == NULL) return NULL;
            return el_hir_new_some_opt_intr(binder->arena, span, to, base);
        }

        return _cast_untyped(binder, span, expr, to);
    }

    if (type_eql(from, to)) return expr;

    ElHirExpr* quals = cast_quals_only(binder, expr, from, to);
    if (quals != NULL) return quals;

    // View types that matched shape but failed mutability — don't fall through.
    if ((is_view_type(from) || is_view_type(to))
        && type_eql_unqual(from, to)
        && !mut_compatible(from, to)) {
        el_diag_report(
            binder->diag, EL_DIAG_ERROR, "sema.invalid-cast", span,
            "invalid cast from '${from}' to '${to}'",
            EL_DIAG_TYPE("from", from), EL_DIAG_TYPE("to", to),
        );
        return NULL;
    }

    return implicit_cast_impl(
        binder, span, expr, from, to,
        el_hir_type_canonical(from),
        el_hir_type_canonical(to)
    );
}

static ElHirExpr* cast_untyped_compound(ElBinder* binder, ElSourceSpan span, ElHirExpr* expr, ElHirType* to) {
    switch (expr->kind) {
    case EL_HIR_EXPR_BINARY: {
        ElHirBinExpr* bin = &expr->as.binary;
        ElHirExpr* left = _el_binder_implicit_cast(binder, bin->left->span, bin->left, to);
        ElHirExpr* right = _el_binder_implicit_cast(binder, bin->right->span, bin->right, to);
        if (left == NULL || right == NULL) return NULL;

        ElHirType* result_type = el_bin_op_is_comparison(bin->op)
            ? binder->builtins->type_bool
            : to;

        ElHirExpr* out = el_hir_new_bin_expr(binder->arena, expr->span, result_type, bin->op, left, right);
        out = _el_binder_simplify_expr(binder, out);

        if (out != NULL && out->type != NULL && !type_eql(out->type, to))
            return _el_binder_implicit_cast(binder, span, out, to);

        return out;
    }
    case EL_HIR_EXPR_UNARY: {
        ElUnaryOp op = expr->as.unary.op;
        if (el_unary_op_is_incdec(op) || op == EL_UNARY_OP_ADDROF || op == EL_UNARY_OP_DEREF || op == EL_UNARY_OP_OPT_UNWRAP) {
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
            span, "untyped expression cannot be converted to type '${to}'",
            EL_DIAG_TYPE("to", to),
        );
    }
}

static ElHirExpr* cast_untyped_null(ElBinder* binder, ElSourceSpan span, ElHirExpr* expr, ElHirType* to, ElHirType* to_c, bool* bad) {
    ElHirLiteral* lit = &expr->as.literal;
    if (to_c->kind == EL_HIR_TYPE_OPT) {
        return el_hir_new_null_opt_intr(binder->arena, expr->span, to);
    } else if (to_c->kind == EL_HIR_TYPE_REF) {
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
        return *bad = true, NULL;
    }

    return NULL;
}

static ElHirExpr* cast_untyped_string(ElBinder* binder, ElSourceSpan span, ElHirExpr* expr, ElHirType* to, ElHirType* to_c, bool* bad) {
    ElHirLiteral* lit = &expr->as.literal;
    ElHirType* type_char = binder->builtins->type_char;

    bool is_slice = (to_c->kind == EL_HIR_TYPE_SLICE && type_eql_unqual(to_c->as.slice.base, type_char))
                 || (to_c->kind == EL_HIR_TYPE_RWSLICE && type_eql_unqual(to_c->as.rwslice.base, type_char));

    if (to_c->kind == EL_HIR_TYPE_ARRAY && type_eql_unqual(to_c->as.array.base, type_char)) {
        if (to_c->as.array.size == lit->of.str_.len) {
            return el_hir_new_string_const(binder->arena, expr->span, to, lit->of.str_, EL_STORAGECLS_STATIC);
        } else {
            *bad = true;
            el_diag_report(
                binder->diag, EL_DIAG_ERROR, "sema.invalid-cast",
                span, "untyped ${of} literal cannot be converted to array type '${to}'",
                EL_DIAG_STRING("of", el_hir_literal_kind_to_string(lit->kind)),
                EL_DIAG_TYPE("to", to),
            );
            el_diag_help(
                binder->diag, "array length mismatch: expected ${expected}, got ${got}",
                EL_DIAG_INT("expected", lit->of.str_.len),
                EL_DIAG_INT("got", to_c->as.array.size)
            );
            if (to_c->as.array.size == lit->of.str_.len + 1) {
                el_diag_help(
                    binder->diag, "string literals in elash are not null terminated by default",
                );
            }
        }
    } else if (is_slice) {
        return _el_binder_implicit_cast(
            binder, expr->span,
            el_hir_new_string_const(binder->arena, expr->span,
                string_lit_type(binder, lit->of.str_.len),
                lit->of.str_, EL_STORAGECLS_STATIC),
            to
        );
    }

    return NULL;
}

// TODO: split this function into smaller helpers
ElHirExpr* _cast_untyped(ElBinder* binder, ElSourceSpan span, ElHirExpr* expr, ElHirType* to) {
    ElHirType* to_c = el_hir_type_canonical(to);
    if (to_c->kind == EL_HIR_TYPE_DISTINCT) {
        if (!_el_binder_ensure_complete(binder, span, to_c))
            return NULL;

        ElHirExpr* casted = _cast_untyped(binder, span, expr, to_c->as.distinct.orig);
        if (casted == NULL) return NULL;
        return el_hir_new_semcast_expr(binder->arena, expr->span, to, casted);
    }

    if (expr->kind != EL_HIR_EXPR_LITERAL)
        return cast_untyped_compound(binder, span, expr, to);

    ElHirLiteral* lit = &expr->as.literal;

    if (to_c->kind == EL_HIR_TYPE_PRIM) {
        ElHirPrimType* prim = &to_c->as.prim;
        switch (lit->kind) {
        case EL_HIR_LITERAL_INT:
            if (prim->kind == EL_HIR_PRIMTYPE_INT) {
                ElInt128 wrapped = _el_binder_wrap_typed_int(binder, expr->span, to_c, lit->of.int_);
                return el_hir_new_int_constant(binder->arena, expr->span, to, wrapped);
            } else if (prim->kind == EL_HIR_PRIMTYPE_FLOAT) {
                return el_hir_new_float_constant(binder->arena, expr->span, to, (double)el_i128_lo(lit->of.int_));
            }
            break;
        case EL_HIR_LITERAL_CHAR:
            if (prim->kind == EL_HIR_PRIMTYPE_INT) {
                ElInt128 wrapped = _el_binder_wrap_typed_int(binder, expr->span, to_c, EL_INT128((int64_t)lit->of.char_));
                return el_hir_new_int_constant(binder->arena, expr->span, to, wrapped);
            }
            break;
        case EL_HIR_LITERAL_BOOL:
            if (prim->kind == EL_HIR_PRIMTYPE_BOOL) {
                return el_hir_new_bool_constant(binder->arena, expr->span, to, lit->of.bool_);
            }
            break;
        case EL_HIR_LITERAL_FLOAT:
            if (prim->kind == EL_HIR_PRIMTYPE_FLOAT) {
                return el_hir_new_float_constant(binder->arena, expr->span, to, lit->of.float_);
            } else if (prim->kind == EL_HIR_PRIMTYPE_INT) {
                ElInt128 wrapped = _el_binder_wrap_typed_int(binder, expr->span, to_c, EL_INT128((int64_t)lit->of.float_));
                return el_hir_new_int_constant(binder->arena, expr->span, to, wrapped);
            }
            break;
        default:
            break; // handled below
        }
    }

    if (lit->kind == EL_HIR_LITERAL_STRING) {
        bool bad = false;
        ElHirExpr* casted = cast_untyped_string(binder, span, expr, to, to_c, &bad);
        HANDLE(casted, bad);
    }

    if (lit->kind == EL_HIR_LITERAL_NULL) {
        bool bad = false;
        ElHirExpr* casted = cast_untyped_null(binder, span, expr, to, to_c, &bad);
        if (casted == NULL) {
            if (bad) return NULL;
        } else {
            return casted;
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
            ElHirType* type = string_lit_type(binder, lit->of.str_.len);
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
