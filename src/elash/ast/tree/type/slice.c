#include <elash/ast/tree/type.h>

ElAstType* el_ast_new_type_slice(ElDynArena* arena, ElSourceSpan span, ElAstType* base, bool is_raw) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstType, {
        .kind = EL_AST_TYPE_SLICE,
        .span = span,
        .as.slice = { base, is_raw },
    });
}
