#include <elash/ast/tree/expr/bin.h>
#include <elash/ast/tree/expr.h>

#include <elash/util/assert.h>

ElAstExpr* el_ast_new_bin_expr(ElDynArena* arena, ElSourceSpan span, ElAstBinOp op, ElAstExpr* left, ElAstExpr* right) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstExpr, {
        .type = EL_AST_EXPR_BINARY,
        .span = span,
        .next = NULL,
        .as.binary = {
            .left = left,
            .op = op,
            .right = right,
        },
    });
}
