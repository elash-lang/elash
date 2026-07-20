#include <elash/ast/tree/type.h>

ElAstType* el_ast_new_type_name(ElDynArena* arena, ElSourceSpan span, ElAstIdent* name) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstType, {
        .kind = EL_AST_TYPE_NAME,
        .span = span,
        .as.name = name,
    });
}

void el_ast_type_list_append(ElAstType** head, ElAstType** tail, ElAstType* type) {
    if (*head == NULL) {
        *head = type;
        *tail = type;
    } else {
        (*tail)->next = type;
        *tail = type;
    }
}
