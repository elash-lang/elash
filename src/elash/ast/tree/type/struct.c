#include <elash/ast/tree/type.h>

ElAstType* el_ast_new_type_struct(ElDynArena* arena, ElSourceSpan span, ElAstDecl* fields, usize count) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstType, {
        .kind = EL_AST_TYPE_STRUCT,
        .span = span,
        .as.struct_ = { fields, count },
    });
}
