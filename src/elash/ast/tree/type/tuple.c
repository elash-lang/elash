#include <elash/ast/tree/type.h>

ElAstType* el_ast_new_type_tuple(ElDynArena* arena, ElSourceSpan span, ElAstTupleElement* head, usize count) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstType, {
        .kind = EL_AST_TYPE_TUPLE,
        .span = span,
        .as.tuple = { head, count },
    });
}

void el_ast_tuple_add_elem(ElAstTupleElement** head, ElAstTupleElement** tail, ElAstTupleElement* element) {
    if (*head == NULL) {
        *head = element;
        *tail = element;
    } else {
        (*tail)->next = element;
        *tail = element;
    }
}
