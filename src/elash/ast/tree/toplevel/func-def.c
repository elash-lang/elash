#include <elash/ast/tree/toplevel.h>

ElAstTopLevelNode* el_ast_new_func_def(ElDynArena* arena, ElSourceSpan span, ElAstFuncSignature sig, ElAstBlockStmtNode* block) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstTopLevelNode, {
        .type = EL_AST_TOPLVL_FUNC_DEF,
        .span = span,
        .next = NULL,
        .as.func_def = {
            .sig = sig,
            .block = block,
        }
    });
}
