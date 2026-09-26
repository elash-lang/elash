#include <elash/ast/tree/decl.h>

ElAstTypedefElem* el_ast_new_typedef_elem(ElDynArena* arena, ElStringView name, ElAstType* target) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstTypedefElem, {
        .name = name,
        .target = target,
        .next = NULL,
    });
}

void el_ast_append_typedef_elem(ElAstTypedefElem** head, ElAstTypedefElem** tail, ElAstTypedefElem* elem) {
    if (*tail != NULL) {
        (*tail)->next = elem;
        *tail = elem;
    } else {
        *head = *tail = elem;
    }
}

ElAstDecl* el_ast_new_typedef(ElDynArena* arena, ElSourceSpan span, ElAstTypedefElem* elements) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstDecl, {
        .type = EL_AST_DECL_TYPEDEF,
        .span = span,
        .as.typedef_ = {
            .elements = elements,
        },
    });
}

