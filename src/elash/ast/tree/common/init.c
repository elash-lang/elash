#include <elash/ast/tree/common/init.h>
#include <elash/ast/tree/expr.h>

ElAstInitializer* el_ast_new_init_expr(ElDynArena* arena, ElAstExprNode* expr) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstInitializer, {
        .kind = EL_AST_INIT_EXPR,
        .span = expr->span,
        .expr = expr,
        .next = NULL,
    });
}
