#include <elash/ast/tree/expr/bin.h>
#include <elash/ast/tree/expr.h>

#include <elash/util/assert.h>

ElAstBinExprNode el_ast_bin_expr(ElAstBinOp op, ElAstExprNode* left, ElAstExprNode* right) {
    return (ElAstBinExprNode) {
        .left = left,
        .op = op,
        .right = right,
    };
}

ElAstExprNode* el_ast_new_bin_expr(ElDynArena* arena, ElSourceSpan span, ElAstBinOp type, ElAstExprNode* left, ElAstExprNode* right) {
    ElAstExprNode* node = EL_DYNARENA_NEW(arena, ElAstExprNode);
    node->type = EL_AST_EXPR_BINARY;
    node->span = span;
    node->as.binary = el_ast_bin_expr(type, left, right);
    node->next = NULL;
    return node;
}
