#include <elash/ast/tree/type.h>

ElAstType* el_ast_new_type_struct(ElDynArena* arena, ElSourceSpan span, ElAstStructField* fields, usize count) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstType, {
        .kind = EL_AST_TYPE_STRUCT,
        .span = span,
        .as.struct_ = { fields, count },
    });
}

void el_ast_struct_add_field(ElAstStructField** head, ElAstStructField** tail, ElAstStructField* field) {
    if (*head == NULL) {
        *head = field;
        *tail = field;
    } else {
        (*tail)->next = field;
        *tail = field;
    }
}
