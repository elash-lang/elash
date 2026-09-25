#include <elash/ast/tree/type.h>

ElAstType* el_ast_new_type_opt(ElDynArena* arena, ElSourceSpan span, ElMutabilitySpec mut, ElAstType* base) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstType, {
        .kind = EL_AST_TYPE_OPT,
        .span = span, .mut = mut,
        .as.opt = { base },
    });
}

