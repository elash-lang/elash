#include <elash/ast/tree/expr/cast.h>
#include <elash/ast/tree/expr.h>

ElAstExpr* el_ast_new_cast_expr(ElDynArena* arena, ElSourceSpan span, ElAstExpr* expr, ElAstType* type) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstExpr, {
        .type = EL_AST_EXPR_CAST,
        .span = span,
        .next = NULL,
        .as.cast = {
            .expr = expr,
            .type = type,
        },
    });
}
