#include <elash/ast/tree/common/ident.h>
#include <elash/ast/tree/expr.h>

ElAstIdent el_ast_ident(ElSourceSpan span, ElStringView name) {
    return (ElAstIdent) {
        .span = span,
        .name = name,
    };
}

ElAstIdent* el_ast_new_ident_raw(ElDynArena* arena, ElSourceSpan span, ElStringView name) {
    ElAstIdent* node = EL_DYNARENA_NEW(arena, ElAstIdent);
    *node = el_ast_ident(span, name);
    return node;
}

ElAstExpr* el_ast_new_ident(ElDynArena* arena, ElSourceSpan span, ElStringView name) {
    ElAstExpr* node = EL_DYNARENA_NEW(arena, ElAstExpr);
    node->type = EL_AST_EXPR_IDENT;
    node->span = span;
    node->as.ident = el_ast_ident(span, name);
    node->next = NULL;
    return node;
}
