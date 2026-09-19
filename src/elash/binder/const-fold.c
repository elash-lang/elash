#include "binder-internals.h"

#include <elash/diag/engine.h>
#include <elash/hir/type/prim.h>
#include <elash/util/assert.h>
#include <elash/util/int128.h>

// NOLINTBEGIN(readability-magic-numbers)

static unsigned prim_int_bits(const ElHirPrimType* prim) {
    switch (prim->as.integral.width) {
    case EL_HIR_IWIDTH_8:         return 8;
    case EL_HIR_IWIDTH_16:        return 16;
    case EL_HIR_IWIDTH_32:        return 32;
    case EL_HIR_IWIDTH_64:        return 64;
    case EL_HIR_IWIDTH_128:       return 128;
    case EL_HIR_IWIDTH_NATIVE:    return 64;
    case EL_HIR_IWIDTH_EFFICIENT: return 32;
    }
    EL_UNREACHABLE_ENUM_VAL(ElHirIntWidth, prim->as.integral.width);
}

static ElUint128 int_bit_mask(unsigned bits) {
    if (bits >= 128) return UINT128_MAX;
    return el_u128_sub(el_u128_shl(EL_UINT128(1), (int)bits), EL_UINT128(1));
}

ElInt128 _el_binder_wrap_typed_int(ElBinder* binder, ElSourceSpan span, ElHirType* type, ElInt128 value) {
    EL_ASSERT(type->kind == EL_HIR_TYPE_PRIM && type->as.prim.kind == EL_PRIMTYPE_INT, "expected integral type");

    const ElHirPrimType* prim = &type->as.prim;
    unsigned bits = prim_int_bits(prim);
    bool overflow;

    if (prim->as.integral.is_signed) {
        ElInt128 min = el_i128_neg(el_i128_shl(EL_INT128(1), (int)bits - 1));
        ElInt128 max = el_i128_sub(el_i128_shl(EL_INT128(1), (int)bits - 1), EL_INT128(1));
        overflow = bits < 128 && (el_i128_lt(value, min) || el_i128_gt(value, max));
    } else {
        overflow = bits < 128 && el_u128_ne(el_u128_shr(el_i128_bitcast_u128(value), (int)bits), EL_UINT128(0));
    }

    if (overflow) {
        el_diag_report(
            binder->diag, EL_DIAG_WARN, "sema.overflow", span,
            "integer overflow in constant expression; result wraps to type ${type}",
            EL_DIAG_TYPE("type", type),
        );
    }

    if (bits >= 128) return value;

    ElUint128 uv = el_u128_and(el_i128_bitcast_u128(value), int_bit_mask(bits));
    if (prim->as.integral.is_signed) {
        ElUint128 sign = el_u128_shl(EL_UINT128(1), (int)bits - 1);
        if (el_u128_ne(el_u128_and(uv, sign), EL_UINT128(0))) {
            uv = el_u128_or(uv, el_u128_not(int_bit_mask(bits)));
        }
    }
    return el_u128_bitcast_i128(uv);
}

// NOLINTEND(readability-magic-numbers)

#define TYPED_INT_RET(type, val, span) \
    el_hir_new_int_constant(binder->arena, span, type, _el_binder_wrap_typed_int(binder, span, type, val))
#define TYPED_CHAR_RET(type, val, span)   el_hir_new_char_constant(binder->arena, span, type, val)
#define TYPED_BOOL_RET(type, val, span)   el_hir_new_bool_constant(binder->arena, span, type, val)
#define TYPED_FLOAT_RET(type, val, span)  el_hir_new_float_constant(binder->arena, span, type, val)

#define UNTYPED_INT_RET(type, val, span)   el_hir_new_int_lit(binder->arena, span, val)
#define UNTYPED_CHAR_RET(type, val, span)  el_hir_new_char_lit(binder->arena, span, val)
#define UNTYPED_BOOL_RET(type, val, span)  el_hir_new_bool_lit(binder->arena, span, val)
#define UNTYPED_FLOAT_RET(type, val, span) el_hir_new_float_lit(binder->arena, span, val)

#define ARITH_BW_BIN_OP_CASES(a, b, RET_MACRO, type, span)                                              \
    case EL_SEMA_BIN_OP_ADD:    return RET_MACRO(type, el_i128_add((a), (b)), span);                    \
    case EL_SEMA_BIN_OP_SUB:    return RET_MACRO(type, el_i128_sub((a), (b)), span);                    \
    case EL_SEMA_BIN_OP_MUL:    return RET_MACRO(type, el_i128_mul((a), (b)), span);                    \
    case EL_SEMA_BIN_OP_DIV:                                                                            \
        if (el_i128_eq((b), EL_INT128(0))) {                                                            \
            el_diag_report(binder->diag, EL_DIAG_ERROR, "sema.div-by-zero", span,                       \
                "division by zero in constant expression");                                             \
            return NULL;                                                                                \
        }                                                                                               \
        return RET_MACRO(type, el_i128_div((a), (b)), span);                                            \
    case EL_SEMA_BIN_OP_MOD:                                                                            \
        if (el_i128_eq((b), EL_INT128(0))) {                                                            \
            el_diag_report(binder->diag, EL_DIAG_ERROR, "sema.div-by-zero", span,                       \
                "division by zero in constant expression");                                             \
            return NULL;                                                                                \
        }                                                                                               \
        return RET_MACRO(type, el_i128_mod((a), (b)), span);                                            \
    case EL_SEMA_BIN_OP_BW_AND: return RET_MACRO(type, el_i128_and((a), (b)), span);                    \
    case EL_SEMA_BIN_OP_BW_OR:  return RET_MACRO(type, el_i128_or((a), (b)), span);                     \
    case EL_SEMA_BIN_OP_BW_XOR: return RET_MACRO(type, el_i128_xor((a), (b)), span);                    \
    case EL_SEMA_BIN_OP_BW_IMP: return RET_MACRO(type, el_i128_or(el_i128_not((a)), (b)), span);        \
    case EL_SEMA_BIN_OP_SHL:    return RET_MACRO(type, el_i128_shl((a), (int)el_u128_lo(el_i128_bitcast_u128(b))), span); \
    case EL_SEMA_BIN_OP_SHR:    return RET_MACRO(type, el_i128_shr((a), (int)el_u128_lo(el_i128_bitcast_u128(b))), span);

#define ARITH_FLOAT_BIN_OP_CASES(a, b, RET_MACRO, type, span)                                       \
    case EL_SEMA_BIN_OP_ADD:    return RET_MACRO(type, (a) + (b), span);                            \
    case EL_SEMA_BIN_OP_SUB:    return RET_MACRO(type, (a) - (b), span);                            \
    case EL_SEMA_BIN_OP_MUL:    return RET_MACRO(type, (a) * (b), span);                            \
    case EL_SEMA_BIN_OP_DIV:    return RET_MACRO(type, (a) / (b), span);

#define COMP_INT_BIN_OP_CASES(a, b, RET_BOOL, type, span)                  \
    case EL_SEMA_BIN_OP_EQ:  return RET_BOOL(type, el_i128_eq((a), (b)), span); \
    case EL_SEMA_BIN_OP_NEQ: return RET_BOOL(type, el_i128_ne((a), (b)), span); \
    case EL_SEMA_BIN_OP_LT:  return RET_BOOL(type, el_i128_lt((a), (b)), span); \
    case EL_SEMA_BIN_OP_LTE: return RET_BOOL(type, el_i128_le((a), (b)), span); \
    case EL_SEMA_BIN_OP_GT:  return RET_BOOL(type, el_i128_gt((a), (b)), span); \
    case EL_SEMA_BIN_OP_GTE: return RET_BOOL(type, el_i128_ge((a), (b)), span);

#define COMP_BIN_OP_CASES(a, b, RET_BOOL, type, span)                 \
    case EL_SEMA_BIN_OP_EQ:  return RET_BOOL(type, (a) == (b), span); \
    case EL_SEMA_BIN_OP_NEQ: return RET_BOOL(type, (a) != (b), span); \
    case EL_SEMA_BIN_OP_LT:  return RET_BOOL(type, (a) < (b), span);  \
    case EL_SEMA_BIN_OP_LTE: return RET_BOOL(type, (a) <= (b), span); \
    case EL_SEMA_BIN_OP_GT:  return RET_BOOL(type, (a) > (b), span);  \
    case EL_SEMA_BIN_OP_GTE: return RET_BOOL(type, (a) >= (b), span);

#define BOOL_BIN_OP_CASES(a, b, RET_BOOL, type, span)                 \
    case EL_SEMA_BIN_OP_EQ:  return RET_BOOL(type, (a) == (b), span); \
    case EL_SEMA_BIN_OP_NEQ: return RET_BOOL(type, (a) != (b), span); \
    case EL_SEMA_BIN_OP_AND: return RET_BOOL(type, (a) && (b), span); \
    case EL_SEMA_BIN_OP_OR:  return RET_BOOL(type, (a) || (b), span); \
    case EL_SEMA_BIN_OP_IMP: return RET_BOOL(type, !(a) || (b), span);

#define UNARY_INT_OP_CASES(a, RET_MACRO, type, span)                       \
    case EL_SEMA_UNARY_OP_POS:    return RET_MACRO(type, (a), span);        \
    case EL_SEMA_UNARY_OP_NEG:    return RET_MACRO(type, el_i128_neg((a)), span); \
    case EL_SEMA_UNARY_OP_BW_NOT: return RET_MACRO(type, el_i128_not((a)), span);

#define UNARY_FLOAT_OP_CASES(a, RET_MACRO, type, span)                \
    case EL_SEMA_UNARY_OP_POS:    return RET_MACRO(type, +(a), span); \
    case EL_SEMA_UNARY_OP_NEG:    return RET_MACRO(type, -(a), span);

#define UNARY_BOOL_OP_CASES(a, RET_BOOL, type, span) \
    if (op == EL_SEMA_UNARY_OP_NOT) {                \
        return RET_BOOL(type, !(a), span);           \
    }

#define FOLD_BINARY(KIND, T, MEMBER, TYPED_RET, UNTYPED_RET)         \
    case EL_PRIMTYPE_##KIND: {                                       \
        T a = lhs->as.constant.as.MEMBER;                            \
        T b = rhs->as.constant.as.MEMBER;                            \
        switch (op) {                                                \
        ARITH_BW_BIN_OP_CASES(a, b, TYPED_RET, lhs->type, lhs->span) \
        COMP_INT_BIN_OP_CASES(a, b, UNTYPED_BOOL_RET, NULL, lhs->span)   \
        default: return NULL;                                        \
        }                                                            \
    }

#define FOLD_BINARY_FLOAT(KIND, T, MEMBER, TYPED_RET, UNTYPED_RET)         \
    case EL_PRIMTYPE_##KIND: {                                             \
        T a = lhs->as.constant.as.MEMBER;                                  \
        T b = rhs->as.constant.as.MEMBER;                                  \
        switch (op) {                                                      \
        ARITH_FLOAT_BIN_OP_CASES(a, b, TYPED_RET, lhs->type, lhs->span)    \
        COMP_BIN_OP_CASES(a, b, UNTYPED_BOOL_RET, NULL, lhs->span)         \
        default: return NULL;                                              \
        }                                                                  \
    }

#define FOLD_UNARY(KIND, T, MEMBER, TYPED_RET, UNTYPED_RET)            \
    case EL_PRIMTYPE_##KIND: {                                         \
        T a = operand->as.constant.as.MEMBER;                          \
        switch (op) {                                                  \
        UNARY_INT_OP_CASES(a, TYPED_RET, operand->type, operand->span) \
        default: return NULL;                                          \
        }                                                              \
    }

#define FOLD_UNARY_FLOAT(KIND, T, MEMBER, TYPED_RET, UNTYPED_RET)      \
    case EL_PRIMTYPE_##KIND: {                                         \
        T a = operand->as.constant.as.MEMBER;                          \
        switch (op) {                                                  \
        UNARY_FLOAT_OP_CASES(a, TYPED_RET, operand->type, operand->span) \
        default: return NULL;                                          \
        }                                                              \
    }

#define FOLD_BINARY_UNTYPED(KIND, T, MEMBER, TYPED_RET, UNTYPED_RET)        \
    if (lkind == EL_HIR_LITERAL_##KIND && rkind == EL_HIR_LITERAL_##KIND) { \
        T a = lhs->as.literal.of.MEMBER;                                \
        T b = rhs->as.literal.of.MEMBER;                                \
        switch (op) {                                                       \
        ARITH_BW_BIN_OP_CASES(a, b, UNTYPED_RET, NULL, lhs->span)           \
        COMP_INT_BIN_OP_CASES(a, b, UNTYPED_BOOL_RET, NULL, lhs->span)          \
        default: return NULL;                                               \
        }                                                                   \
    }

#define FOLD_BINARY_UNTYPED_FLOAT(KIND, T, MEMBER, TYPED_RET, UNTYPED_RET)  \
    if (lkind == EL_HIR_LITERAL_##KIND && rkind == EL_HIR_LITERAL_##KIND) { \
        T a = lhs->as.literal.of.MEMBER;                                \
        T b = rhs->as.literal.of.MEMBER;                                \
        switch (op) {                                                       \
        ARITH_FLOAT_BIN_OP_CASES(a, b, UNTYPED_RET, NULL, lhs->span)        \
        COMP_BIN_OP_CASES(a, b, UNTYPED_BOOL_RET, NULL, lhs->span)          \
        default: return NULL;                                               \
        }                                                                   \
    }

#define FOLD_UNARY_UNTYPED(KIND, T, MEMBER, TYPED_RET, UNTYPED_RET) \
    if (kind == EL_HIR_LITERAL_##KIND) {                            \
        T a = operand->as.literal.of.MEMBER;                    \
        switch (op) {                                               \
        UNARY_INT_OP_CASES(a, UNTYPED_RET, NULL, operand->span)     \
        default: return NULL;                                       \
        }                                                           \
    }

#define FOLD_UNARY_UNTYPED_FLOAT(KIND, T, MEMBER, TYPED_RET, UNTYPED_RET) \
    if (kind == EL_HIR_LITERAL_##KIND) {                                  \
        T a = operand->as.literal.of.MEMBER;                          \
        switch (op) {                                                     \
        UNARY_FLOAT_OP_CASES(a, UNTYPED_RET, NULL, operand->span)         \
        default: return NULL;                                             \
        }                                                                 \
    }

// i love X-macros
#define EL_FOR_EACH_INTEGRAL_TYPE(X) \
    X(INT, ElInt128, int_, TYPED_INT_RET, UNTYPED_INT_RET)

#define EL_FOR_EACH_FLOAT_TYPE(X) \
    X(FLOAT, double, float_, TYPED_FLOAT_RET, UNTYPED_FLOAT_RET)

// NOLINTNEXTLINE(readability-function-cognitive-complexity): clang-tidy is so stupid that it don't understand macros ig
static ElHirExpr* apply_binary_operator(ElBinder* binder, ElHirExpr* lhs, ElBinOp op, ElHirExpr* rhs) {
    switch (lhs->type->as.prim.kind) {
        EL_FOR_EACH_INTEGRAL_TYPE(FOLD_BINARY);
        EL_FOR_EACH_FLOAT_TYPE(FOLD_BINARY_FLOAT);
        case EL_PRIMTYPE_BOOL: {
            bool a = lhs->as.constant.as.bool_;
            bool b = rhs->as.constant.as.bool_;
            switch (op) {
            BOOL_BIN_OP_CASES(a, b, TYPED_BOOL_RET, binder->builtins->type_bool, lhs->span);
            default: return NULL;
            }
        }
        default: return NULL;
    }
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity): same reason as before
static ElHirExpr* apply_unary_operator(ElBinder* binder, ElUnaryOp op, ElHirExpr* operand) {
    switch (operand->type->as.prim.kind) {
    EL_FOR_EACH_INTEGRAL_TYPE(FOLD_UNARY);
    EL_FOR_EACH_FLOAT_TYPE(FOLD_UNARY_FLOAT);
    case EL_PRIMTYPE_BOOL: {
        bool a = operand->as.constant.as.bool_;
        UNARY_BOOL_OP_CASES(a, TYPED_BOOL_RET, binder->builtins->type_bool, operand->span);
        return NULL;
    }
    default: return NULL;
    }
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity): also same reason as before
static ElHirExpr* apply_binary_operator_untyped(ElBinder* binder, ElHirExpr* lhs, ElBinOp op, ElHirExpr* rhs) {
    ElHirLiteralKind lkind = lhs->as.literal.kind;
    ElHirLiteralKind rkind = rhs->as.literal.kind;

    EL_FOR_EACH_INTEGRAL_TYPE(FOLD_BINARY_UNTYPED);
    EL_FOR_EACH_FLOAT_TYPE(FOLD_BINARY_UNTYPED_FLOAT);

    if (lkind == EL_HIR_LITERAL_BOOL && rkind == EL_HIR_LITERAL_BOOL) {
        bool a = lhs->as.literal.of.bool_;
        bool b = rhs->as.literal.of.bool_;
        switch (op) {
        BOOL_BIN_OP_CASES(a, b, UNTYPED_BOOL_RET, NULL, lhs->span);
        default: return NULL;
        }
    }
    return NULL;
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity): also also same reason as before
static ElHirExpr* apply_unary_operator_untyped(ElBinder* binder, ElUnaryOp op, ElHirExpr* operand) {
    ElHirLiteralKind kind = operand->as.literal.kind;
    EL_FOR_EACH_INTEGRAL_TYPE(FOLD_UNARY_UNTYPED);
    EL_FOR_EACH_FLOAT_TYPE(FOLD_UNARY_UNTYPED_FLOAT);
    if (kind == EL_HIR_LITERAL_BOOL) {
        bool a = operand->as.literal.of.bool_;
        UNARY_BOOL_OP_CASES(a, UNTYPED_BOOL_RET, NULL, operand->span);
    }
    return NULL;
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity): it's all good
ElHirExpr* _el_binder_simplify_expr(ElBinder* binder, ElHirExpr* expr) {
    if (expr == NULL) return NULL;

    ElHirType* type = expr->type;
    if (type != NULL && type->kind == EL_HIR_TYPE_DISTINCT) {
        type = el_hir_type_unwrap_distinct(type);
    }

    if (type != NULL && type->kind != EL_HIR_TYPE_PRIM) return expr;

    switch (expr->kind) {
    case EL_HIR_EXPR_BINARY: {
        ElHirBinExpr* bin = &expr->as.binary;
        bin->left  = _el_binder_simplify_expr(binder, bin->left);
        bin->right = _el_binder_simplify_expr(binder, bin->right);

        if (bin->left->kind == EL_HIR_EXPR_LITERAL && bin->right->kind == EL_HIR_EXPR_LITERAL) {
            ElHirExpr* res = apply_binary_operator_untyped(binder, bin->left, bin->op, bin->right);
            if (res != NULL) return res;
        } else if (bin->left->kind == EL_HIR_EXPR_CONST && bin->right->kind == bin->left->kind) {
            ElHirExpr* res = apply_binary_operator(binder, bin->left, bin->op, bin->right);
            if (res != NULL) return res;
        }
        return expr;
    }
    case EL_HIR_EXPR_UNARY: {
        ElHirUnaryExpr* unary = &expr->as.unary;
        unary->operand = _el_binder_simplify_expr(binder, unary->operand);
        if (unary->operand->kind == EL_HIR_EXPR_LITERAL) {
            ElHirExpr* res = apply_unary_operator_untyped(binder, unary->op, unary->operand);
            if (res != NULL) return res;
        } else if (unary->operand->kind == EL_HIR_EXPR_CONST) {
            ElHirExpr* res = apply_unary_operator(binder, unary->op, unary->operand);
            if (res != NULL) return res;
        }
        return expr;
    }
    case EL_HIR_EXPR_AGGINIT: {
        ElHirAggInit* arr = &expr->as.agginit;
        for (usize i = 0; i < arr->count; ++i) {
            arr->values[i] = _el_binder_simplify_expr(binder, arr->values[i]);
        }
        return expr;
    }
    case EL_HIR_EXPR_CAST: {
        ElHirCastExpr* cast = &expr->as.cast;
        cast->expr = _el_binder_simplify_expr(binder, cast->expr);

        EL_ASSERT(cast->expr != NULL, "simplify expr returned null");
        if (cast->expr->kind == EL_HIR_EXPR_CONST || cast->expr->kind == EL_HIR_EXPR_LITERAL) {
            ElHirExpr* folded = _el_binder_eval_const_cast(binder, expr->span, cast->expr, expr->type);
            if (folded != NULL) return folded;
        }

        return expr;
    }
    case EL_HIR_EXPR_CALL: {
        ElHirCallExpr* call = &expr->as.call;
        call->callee = _el_binder_simplify_expr(binder, call->callee);
        for (usize i = 0; i < call->arg_count; ++i) {
            call->args[i] = _el_binder_simplify_expr(binder, call->args[i]);
        }
        return expr;
    }
    default:
        return expr;
    }
}
