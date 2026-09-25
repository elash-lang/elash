#include <elash/ast/tree/type.h>

ElAstType* el_ast_new_type_tuple(ElDynArena* arena, ElSourceSpan span, ElMutabilitySpec mut, ElAstType* head, usize count) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstType, {
        .kind = EL_AST_TYPE_TUPLE,
        .span = span, .mut = mut,
        .as.tuple = { head, count },
    });
}
