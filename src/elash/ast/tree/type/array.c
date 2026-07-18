#include <elash/ast/tree/type.h>

ElAstType* el_ast_new_type_array(ElDynArena* arena, ElSourceSpan span, ElAstType* base, ElAstExpr* size) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstType, {
        .kind = EL_AST_TYPE_ARRAY,
        .span = span,
        .as.array = { base, size },
    });
}
