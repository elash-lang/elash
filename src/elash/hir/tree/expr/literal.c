#include <elash/hir/tree/expr/literal.h>
#include <elash/hir/tree/expr.h>

ElHirExprNode* el_hir_new_int_literal(ElDynArena* arena, ElType* type, int64_t value) {
    ElHirExprNode* node = EL_DYNARENA_NEW(arena, ElHirExprNode);
    node->kind = EL_HIR_EXPR_LITERAL;
    node->type = type;
    node->as.literal.as.int_ = value;
    return node;
}

ElHirExprNode* el_hir_new_uint_literal(ElDynArena* arena, ElType* type, uint64_t value) {
    ElHirExprNode* node = EL_DYNARENA_NEW(arena, ElHirExprNode);
    node->kind = EL_HIR_EXPR_LITERAL;
    node->type = type;
    node->as.literal.as.uint_ = value;
    return node;
}

ElHirExprNode* el_hir_new_char_literal(ElDynArena* arena, ElType* type, char value) {
    ElHirExprNode* node = EL_DYNARENA_NEW(arena, ElHirExprNode);
    node->kind = EL_HIR_EXPR_LITERAL;
    node->type = type;
    node->as.literal.as.char_ = value;
    return node;
}

ElHirExprNode* el_hir_new_bool_literal(ElDynArena* arena, ElType* type, bool value) {
    ElHirExprNode* node = EL_DYNARENA_NEW(arena, ElHirExprNode);
    node->kind = EL_HIR_EXPR_LITERAL;
    node->type = type;
    node->as.literal.as.bool_ = value;
    return node;
}
