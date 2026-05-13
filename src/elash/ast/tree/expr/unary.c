#include <elash/ast/tree/expr/unary.h>
#include <elash/ast/tree/expr.h>

#include <elash/util/assert.h>

ElAstExprNode* el_ast_new_unary_expr(ElDynArena* arena, ElSourceSpan span, ElAstUnaryOp op, ElAstExprNode* operand) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstExprNode, {
        .type = EL_AST_EXPR_UNARY,
        .next = NULL,
        .span = span,
        .as.unary = {
            .op = op,
            .operand = operand,
        },
    });
}
