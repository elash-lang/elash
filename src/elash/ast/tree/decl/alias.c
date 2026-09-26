#include <elash/ast/tree/decl/alias.h>
#include <elash/ast/tree/decl.h>

ElAstAliasElem* el_ast_new_alias_elem(ElDynArena* arena, ElStringView name, ElAstToE* target) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstAliasElem, {
        .name = name,
        .target = target,
        .next = NULL,
    });
}

void el_ast_append_alias_elem(ElAstAliasElem** head, ElAstAliasElem** tail, ElAstAliasElem* elem) {
    if (*tail != NULL) {
        (*tail)->next = elem;
        *tail = elem;
    } else {
        *head = *tail = elem;
    }
}

ElAstDecl* el_ast_new_alias(ElDynArena* arena, ElSourceSpan span, ElAstAliasElem* elements) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstDecl, {
        .type = EL_AST_DECL_ALIAS,
        .span = span,
        .as.alias = {
            .elements = elements,
        },
    });
}
