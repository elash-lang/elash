#include <elash/ast/tree/expr/unary.h>
#include <elash/ast/tree/expr.h>

#include <elash/util/assert.h>

ElAstUnaryExprNode el_ast_unary_expr(ElAstUnaryOp op, ElAstExprNode* operand) {
    return (ElAstUnaryExprNode) {
        .op = op,
        .operand = operand,
    };
}

ElAstExprNode* el_ast_new_unary_expr(ElDynArena* arena, ElSourceSpan span, ElAstUnaryOp type, ElAstExprNode* operand) {
    ElAstExprNode* node = EL_DYNARENA_NEW(arena, ElAstExprNode);
    node->type = EL_AST_EXPR_UNARY;
    node->span = span;
    node->as.unary = el_ast_unary_expr(type, operand);
    node->next = NULL;
    return node;
}
