#include <elash/ast/tree/common/init.h>

ElAstInitializer* el_ast_new_init_list(ElDynArena* arena, ElSourceSpan span, ElAstInitializer* head, usize count) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstInitializer, {
        .kind = EL_AST_INIT_LIST,
        .span = span,
        .list.head = head,
        .list.count = count,
        .next = NULL,
    });
}

void el_ast_init_list_append(ElAstInitializer** head, ElAstInitializer** tail, ElAstInitializer* init) {
    if (*tail) {
        (*tail)->next = init;
        *tail = init;
    } else {
        *head = *tail = init;
    }
}
