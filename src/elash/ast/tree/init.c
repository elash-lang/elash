#include <elash/ast/tree/init.h>
#include <elash/ast/tree/expr.h>

ElAstInit* el_ast_new_init_expr(ElDynArena* arena, ElAstExpr* expr) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstInit, {
        .kind = EL_AST_INIT_EXPR,
        .span = expr->span,
        .expr = expr,
        .next = NULL,
    });
}
