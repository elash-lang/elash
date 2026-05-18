#include <elash/ast/tree/common/init.h>

ElAstInit* el_ast_new_init_list(ElDynArena* arena, ElSourceSpan span, ElAstInit* head, usize count) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstInit, {
        .kind = EL_AST_INIT_LIST,
        .span = span,
        .list.head = head,
        .list.count = count,
        .next = NULL,
    });
}

void el_ast_init_list_append(ElAstInit** head, ElAstInit** tail, ElAstInit* init) {
    if (*tail) {
        (*tail)->next = init;
        *tail = init;
    } else {
        *head = *tail = init;
    }
}
