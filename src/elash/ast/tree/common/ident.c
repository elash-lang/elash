#include <elash/ast/tree/common/ident.h>
#include <elash/ast/tree/expr.h>

ElAstIdentNode el_ast_ident_node(ElSourceSpan span, ElStringView name) {
    return (ElAstIdentNode) {
        .span = span,
        .name = name,
    };
}

ElAstIdentNode* el_ast_new_ident_node_raw(ElDynArena* arena, ElSourceSpan span, ElStringView name) {
    ElAstIdentNode* node = EL_DYNARENA_NEW(arena, ElAstIdentNode);
    *node = el_ast_ident_node(span, name);
    return node;
}

ElAstExprNode* el_ast_new_ident_node(ElDynArena* arena, ElSourceSpan span, ElStringView name) {
    ElAstExprNode* node = EL_DYNARENA_NEW(arena, ElAstExprNode);
    node->type = EL_AST_EXPR_IDENT;
    node->span = span;
    node->as.ident = el_ast_ident_node(span, name);
    node->next = NULL;
    return node;
}
