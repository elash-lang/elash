#include <elash/ast/toplevel.h>

ElAstTopLevelNode* el_ast_new_func_decl(ElDynArena* arena, ElSourceSpan span, ElAstFuncSignature sig) {
    ElAstTopLevelNode* node = EL_DYNARENA_NEW(arena, ElAstTopLevelNode);
    *node = (ElAstTopLevelNode) {
        .type = EL_AST_TOPLVL_FUNC_DECL,
        .span = span,
        .next = NULL,
        .as.func_decl = {
            .sig = sig,
        }
    };
    return node;
}
