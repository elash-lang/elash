#include <elash/binder/binder.h>

#define TYPED_INT_RET(type, val, span)    el_hir_new_int_constant(binder->hir_arena, span, type, val)
#define TYPED_CHAR_RET(type, val, span)   el_hir_new_char_constant(binder->hir_arena, span, type, val)
#define TYPED_BOOL_RET(type, val, span)   el_hir_new_bool_constant(binder->hir_arena, span, type, val)

#define UNTYPED_INT_RET(type, val, span)  el_hir_new_untyped_int_lit(binder->hir_arena, span, val)
#define UNTYPED_CHAR_RET(type, val, span) el_hir_new_untyped_char_lit(binder->hir_arena, span, val)
#define UNTYPED_BOOL_RET(type, val, span) el_hir_new_untyped_bool_lit(binder->hir_arena, span, val)

// TODO: we probably should report an error on division/modulo by zero instead of returning NULL
#define ARITH_BW_BIN_OP_CASES(a, b, RET_MACRO, type, span)                                          \
    case EL_SEMA_BIN_OP_ADD:    return RET_MACRO(type, (a) + (b), span);                            \
    case EL_SEMA_BIN_OP_SUB:    return RET_MACRO(type, (a) - (b), span);                            \
    case EL_SEMA_BIN_OP_MUL:    return RET_MACRO(type, (a) * (b), span);                            \
    case EL_SEMA_BIN_OP_DIV:    if ((b) == 0) return NULL; return RET_MACRO(type, (a) / (b), span); \
    case EL_SEMA_BIN_OP_MOD:    if ((b) == 0) return NULL; return RET_MACRO(type, (a) % (b), span); \
    case EL_SEMA_BIN_OP_BW_AND: return RET_MACRO(type, (a) & (b), span);                            \
    case EL_SEMA_BIN_OP_BW_OR:  return RET_MACRO(type, (a) | (b), span);                            \
    case EL_SEMA_BIN_OP_BW_XOR: return RET_MACRO(type, (a) ^ (b), span);                            \
    case EL_SEMA_BIN_OP_BW_IMP: return RET_MACRO(type, ~(a) | (b), span);                           \
    case EL_SEMA_BIN_OP_SHL:    return RET_MACRO(type, (a) << (b), span);                           \
    case EL_SEMA_BIN_OP_SHR:    return RET_MACRO(type, (a) >> (b), span);

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

#define UNARY_INT_OP_CASES(a, RET_MACRO, type, span)                  \
    case EL_SEMA_UNARY_OP_POS:    return RET_MACRO(type, +(a), span); \
    case EL_SEMA_UNARY_OP_NEG:    return RET_MACRO(type, -(a), span); \
    case EL_SEMA_UNARY_OP_BW_NOT: return RET_MACRO(type, ~(a), span);

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
        COMP_BIN_OP_CASES(a, b, UNTYPED_BOOL_RET, NULL, lhs->span)   \
        default: return NULL;                                        \
        }                                                            \
    }

#define FOLD_UNARY(KIND, T, MEMBER, TYPED_RET, UNTYPED_RET)            \
    case EL_PRIMTYPE_##KIND: {                                         \
        T a = operand->as.constant.as.MEMBER;                          \
        switch (op) {                                                  \
        UNARY_INT_OP_CASES(a, TYPED_RET, operand->type, operand->span) \
        default: return NULL;                                          \
        }                                                              \
    }

#define FOLD_BINARY_UNTYPED(KIND, T, MEMBER, TYPED_RET, UNTYPED_RET)        \
    if (lkind == EL_HIR_UNTYPED_##KIND && rkind == EL_HIR_UNTYPED_##KIND) { \
        T a = lhs->as.untyped_lit.of.MEMBER;                                \
        T b = rhs->as.untyped_lit.of.MEMBER;                                \
        switch (op) {                                                       \
        ARITH_BW_BIN_OP_CASES(a, b, UNTYPED_RET, NULL, lhs->span)           \
        COMP_BIN_OP_CASES(a, b, UNTYPED_BOOL_RET, NULL, lhs->span)          \
        default: return NULL;                                               \
        }                                                                   \
    }

#define FOLD_UNARY_UNTYPED(KIND, T, MEMBER, TYPED_RET, UNTYPED_RET) \
    if (kind == EL_HIR_UNTYPED_##KIND) {                            \
        T a = operand->as.untyped_lit.of.MEMBER;                    \
        switch (op) {                                               \
        UNARY_INT_OP_CASES(a, UNTYPED_RET, NULL, operand->span)     \
        default: return NULL;                                       \
        }                                                           \
    }

// i love X-macros
#define EL_FOR_EACH_INTEGRAL_TYPE(X)                           \
    X(INT,  int64_t, int_,  TYPED_INT_RET,  UNTYPED_INT_RET)   \
    X(CHAR, char,    char_, TYPED_CHAR_RET, UNTYPED_CHAR_RET);

// NOLINTNEXTLINE(readability-function-cognitive-complexity): clang-tidy is so stupid that it don't understand macros ig
static ElHirExpr* apply_binary_operator(ElBinder* binder, ElHirExpr* lhs, ElSemaBinOp op, ElHirExpr* rhs) {
    switch (lhs->type->as.prim.kind) {
        EL_FOR_EACH_INTEGRAL_TYPE(FOLD_BINARY);
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
static ElHirExpr* apply_unary_operator(ElBinder* binder, ElSemaUnaryOp op, ElHirExpr* operand) {
    switch (operand->type->as.prim.kind) {
    EL_FOR_EACH_INTEGRAL_TYPE(FOLD_UNARY);
    case EL_PRIMTYPE_BOOL: {
        bool a = operand->as.constant.as.bool_;
        UNARY_BOOL_OP_CASES(a, TYPED_BOOL_RET, binder->builtins->type_bool, operand->span);
        return NULL;
    }
    default: return NULL;
    }
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity): also same reason as before
static ElHirExpr* apply_binary_operator_untyped(ElBinder* binder, ElHirExpr* lhs, ElSemaBinOp op, ElHirExpr* rhs) {
    ElHirUntypedLitKind lkind = lhs->as.untyped_lit.kind;
    ElHirUntypedLitKind rkind = rhs->as.untyped_lit.kind;

    EL_FOR_EACH_INTEGRAL_TYPE(FOLD_BINARY_UNTYPED);

    if (lkind == EL_HIR_UNTYPED_BOOL && rkind == EL_HIR_UNTYPED_BOOL) {
        bool a = lhs->as.untyped_lit.of.bool_;
        bool b = rhs->as.untyped_lit.of.bool_;
        switch (op) {
        BOOL_BIN_OP_CASES(a, b, UNTYPED_BOOL_RET, NULL, lhs->span);
        default: return NULL;
        }
    }
    return NULL;
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity): also also same reason as before
static ElHirExpr* apply_unary_operator_untyped(ElBinder* binder, ElSemaUnaryOp op, ElHirExpr* operand) {
    ElHirUntypedLitKind kind = operand->as.untyped_lit.kind;
    EL_FOR_EACH_INTEGRAL_TYPE(FOLD_UNARY_UNTYPED);
    if (kind == EL_HIR_UNTYPED_BOOL) {
        bool a = operand->as.untyped_lit.of.bool_;
        UNARY_BOOL_OP_CASES(a, UNTYPED_BOOL_RET, NULL, operand->span);
    }
    return NULL;
}

ElHirExpr* _el_binder_simplify_expr(ElBinder* binder, ElHirExpr* expr) {
    if (expr == NULL) return NULL;
    if (expr->type != NULL && expr->type->kind != EL_HIR_TYPE_PRIM) return expr;

    switch (expr->kind) {
    case EL_HIR_EXPR_BINARY: {
        ElHirBinExpr* bin = &expr->as.binary;
        bin->left  = _el_binder_simplify_expr(binder, bin->left);
        bin->right = _el_binder_simplify_expr(binder, bin->right);

        if (bin->left->kind == EL_HIR_EXPR_UNTYPEDLIT && bin->right->kind == EL_HIR_EXPR_UNTYPEDLIT) {
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
        if (unary->operand->kind == EL_HIR_EXPR_UNTYPEDLIT) {
            ElHirExpr* res = apply_unary_operator_untyped(binder, unary->op, unary->operand);
            if (res != NULL) return res;
        } else if (unary->operand->kind == EL_HIR_EXPR_CONST) {
            ElHirExpr* res = apply_unary_operator(binder, unary->op, unary->operand);
            if (res != NULL) return res;
        }
        return expr;
    }
    default:
        // TODO: recursively call simplify on all array elements and function arguments
        return expr;
    }
}
