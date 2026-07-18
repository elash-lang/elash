#include <elash/ast/tree/type.h>

ElAstType* el_ast_new_type_name(ElDynArena* arena, ElSourceSpan span, ElAstIdent* name) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstType, {
        .kind = EL_AST_TYPE_NAME,
        .span = span,
        .as.name = name,
    });
}
