#include <elash/ast/tree/expr.h>

ElAstExprNode* el_ast_new_array_lit(ElDynArena* arena, ElSourceSpan span, ElAstTypeNode* type, ElAstInitializer* init) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstExprNode, {
        .type = EL_AST_EXPR_ARRAY_LITERAL,
        .span = span,
        .next = NULL,
        .as.array_lit = {
            .type = type,
            .init = init,
        },
    });
}

