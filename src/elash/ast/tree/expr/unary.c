#include <elash/ast/tree/expr/unary.h>
#include <elash/ast/tree/expr.h>

#include <elash/util/assert.h>

ElAstExpr* el_ast_new_unary_expr(ElDynArena* arena, ElSourceSpan span, ElAstUnaryOp op, ElAstExpr* operand) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstExpr, {
        .type = EL_AST_EXPR_UNARY,
        .next = NULL,
        .span = span,
        .as.unary = {
            .op = op,
            .operand = operand,
        },
    });
}
