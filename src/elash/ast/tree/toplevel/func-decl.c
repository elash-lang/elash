#include <elash/ast/tree/toplevel.h>

ElAstTopLevelNode* el_ast_new_func_decl(ElDynArena* arena, ElSourceSpan span, ElAstFuncSignature sig) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstTopLevelNode, {
        .type = EL_AST_TOPLVL_FUNC_DECL,
        .span = span,
        .next = NULL,
        .as.func_decl = {
            .sig = sig,
        }
    });
}
