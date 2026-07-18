#include <elash/ast/tree/type.h>

ElAstType* el_ast_new_type_ref(ElDynArena* arena, ElSourceSpan span, ElAstType* base) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstType, {
        .kind = EL_AST_TYPE_REF,
        .span = span,
        .as.ref = { base },
    });
}
