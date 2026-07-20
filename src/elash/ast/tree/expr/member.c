#include <elash/ast/tree/expr/member.h>
#include <elash/ast/tree/expr.h>

ElAstExpr* el_ast_new_member_expr(ElDynArena* arena, ElSourceSpan span, ElAstExpr* expr, ElAstIdent* name) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstExpr, {
        .type = EL_AST_EXPR_MEMBER,
        .span = span,
        .next = NULL,
        .as.member = {
            .expr = expr,
            .name = name,
        },
    });
}

ElAstExpr* el_ast_new_tmember_expr(ElDynArena* arena, ElSourceSpan span, ElAstExpr* expr, usize index, ElSourceSpan index_span) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstExpr, {
        .type = EL_AST_EXPR_TMEMBER,
        .span = span,
        .next = NULL,
        .as.tmember = {
            .expr = expr,
            .index = index,
            .index_span = index_span,
        },
    });
}
