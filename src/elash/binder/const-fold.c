#include <elash/binder/binder.h>

// TODO: we probably should report an error on division/modulo by zero instead of returning NULL
#define ARITH_BW_BIN_OP_CASES(a, b, NEW_LIT)                                                                     \
    case EL_SEMA_BIN_OP_ADD: return NEW_LIT(binder->hir_arena, lhs->type, (a) + (b));                            \
    case EL_SEMA_BIN_OP_SUB: return NEW_LIT(binder->hir_arena, lhs->type, (a) - (b));                            \
    case EL_SEMA_BIN_OP_MUL: return NEW_LIT(binder->hir_arena, lhs->type, (a) * (b));                            \
    case EL_SEMA_BIN_OP_DIV: if ((b) == 0) return NULL; return NEW_LIT(binder->hir_arena, lhs->type, (a) / (b)); \
    case EL_SEMA_BIN_OP_MOD: if ((b) == 0) return NULL; return NEW_LIT(binder->hir_arena, lhs->type, (a) % (b)); \
    case EL_SEMA_BIN_OP_BW_AND: return NEW_LIT(binder->hir_arena, lhs->type, (a) & (b));                         \
    case EL_SEMA_BIN_OP_BW_OR:  return NEW_LIT(binder->hir_arena, lhs->type, (a) | (b));                         \
    case EL_SEMA_BIN_OP_BW_XOR: return NEW_LIT(binder->hir_arena, lhs->type, (a) ^ (b));                         \
    case EL_SEMA_BIN_OP_BW_IMP: return NEW_LIT(binder->hir_arena, lhs->type, ~(a) | (b));                        \
    case EL_SEMA_BIN_OP_SHL:    return NEW_LIT(binder->hir_arena, lhs->type, (a) << (b));                        \
    case EL_SEMA_BIN_OP_SHR:    return NEW_LIT(binder->hir_arena, lhs->type, (a) >> (b));

#define COMP_BIN_OP_CASES(a, b) \
    case EL_SEMA_BIN_OP_EQ:  return el_hir_new_bool_literal(binder->hir_arena, binder->builtins->type_bool, (a) == (b)); \
    case EL_SEMA_BIN_OP_NEQ: return el_hir_new_bool_literal(binder->hir_arena, binder->builtins->type_bool, (a) != (b)); \
    case EL_SEMA_BIN_OP_LT:  return el_hir_new_bool_literal(binder->hir_arena, binder->builtins->type_bool, (a) < (b));  \
    case EL_SEMA_BIN_OP_LTE: return el_hir_new_bool_literal(binder->hir_arena, binder->builtins->type_bool, (a) <= (b)); \
    case EL_SEMA_BIN_OP_GT:  return el_hir_new_bool_literal(binder->hir_arena, binder->builtins->type_bool, (a) > (b));  \
    case EL_SEMA_BIN_OP_GTE: return el_hir_new_bool_literal(binder->hir_arena, binder->builtins->type_bool, (a) >= (b));

#define FOLD_BINARY(KIND, T, MEMBER, NEW_LIT)    \
    case EL_PRIMTYPE_##KIND: {                   \
        T a = lhs->as.literal.as.MEMBER;         \
        T b = rhs->as.literal.as.MEMBER;         \
        switch (op) {                            \
        ARITH_BW_BIN_OP_CASES(a, b, NEW_LIT)     \
        COMP_BIN_OP_CASES(a, b)                  \
        default: return NULL;                    \
        }                                        \
    }

#define UNARY_INT_OP_CASES(a, NEW_LIT)                                                   \
    case EL_SEMA_UNARY_OP_POS: return NEW_LIT(binder->hir_arena, operand->type, +(a));   \
    case EL_SEMA_UNARY_OP_NEG: return NEW_LIT(binder->hir_arena, operand->type, -(a));   \
    case EL_SEMA_UNARY_OP_BW_NOT: return NEW_LIT(binder->hir_arena, operand->type, ~(a));

#define FOLD_UNARY(KIND, T, MEMBER, NEW_LIT) \
    case EL_PRIMTYPE_##KIND: {               \
        T a = operand->as.literal.as.MEMBER; \
        switch (op) {                        \
        UNARY_INT_OP_CASES(a, NEW_LIT)       \
        default: return NULL;                \
        }                                    \
    }

// i love X-macros
#define EL_FOR_EACH_INTEGER_TYPE(X)                   \
    X(INT,  int64_t,  int_,  el_hir_new_int_literal)  \
    X(CHAR, char,     char_, el_hir_new_char_literal)

// NOLINTNEXTLINE(readability-function-cognitive-complexity): clang-tidy is so stupid that it don't understand macros ig
static ElHirExpr* apply_binary_operator(ElBinder* binder, ElHirExpr* lhs, ElSemaBinOp op, ElHirExpr* rhs) {
    switch (lhs->type->as.prim.kind) {
        EL_FOR_EACH_INTEGER_TYPE(FOLD_BINARY)
        case EL_PRIMTYPE_BOOL: {
            bool a = lhs->as.literal.as.bool_;
            bool b = rhs->as.literal.as.bool_;
            switch (op) {
            case EL_SEMA_BIN_OP_EQ:  return el_hir_new_bool_literal(binder->hir_arena, binder->builtins->type_bool, a == b);
            case EL_SEMA_BIN_OP_NEQ: return el_hir_new_bool_literal(binder->hir_arena, binder->builtins->type_bool, a != b);
            case EL_SEMA_BIN_OP_AND: return el_hir_new_bool_literal(binder->hir_arena, binder->builtins->type_bool, a && b);
            case EL_SEMA_BIN_OP_OR:  return el_hir_new_bool_literal(binder->hir_arena, binder->builtins->type_bool, a || b);
            case EL_SEMA_BIN_OP_IMP: return el_hir_new_bool_literal(binder->hir_arena, binder->builtins->type_bool, !a || b);
            default: return NULL;
            }
        }
        default: return NULL;
    }
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity): same reason as before
static ElHirExpr* apply_unary_operator(ElBinder* binder, ElSemaUnaryOp op, ElHirExpr* operand) {
    switch (operand->type->as.prim.kind) {
    EL_FOR_EACH_INTEGER_TYPE(FOLD_UNARY)
    case EL_PRIMTYPE_BOOL:
        if (op == EL_SEMA_UNARY_OP_NOT) {
            return el_hir_new_bool_literal(binder->hir_arena, binder->builtins->type_bool, !operand->as.literal.as.bool_);
        }
        return NULL;
    default: return NULL;
    }
}

ElHirExpr* _el_binder_simplify_expr(ElBinder* binder, ElHirExpr* expr) {
    if (expr->type->kind != EL_TYPE_PRIM) return expr;

    switch (expr->kind) {
    case EL_HIR_EXPR_BINARY: {
        ElHirBinExpr* bin = &expr->as.binary;
        bin->left  = _el_binder_simplify_expr(binder, bin->left);
        bin->right = _el_binder_simplify_expr(binder, bin->right);

        if (bin->left->kind == EL_HIR_EXPR_LITERAL && bin->right->kind == bin->left->kind) {
            ElHirExpr* res = apply_binary_operator(binder, bin->left, bin->op, bin->right);
            if (res != NULL) return res;
        }
        return expr;
    }
    case EL_HIR_EXPR_UNARY: {
        ElHirUnaryExpr* unary = &expr->as.unary;
        unary->operand = _el_binder_simplify_expr(binder, unary->operand);
        if (unary->operand->kind == EL_HIR_EXPR_LITERAL) {
            ElHirExpr* res = apply_unary_operator(binder, unary->op, unary->operand);
            if (res != NULL) return res;
        }
        return expr;
    }
    default:
        // TODO: maybe we can iterate over all elements in array literal of all arguments in function call and call simplify on it? but i'm too lazy to do it so
        return expr;
    }
}
