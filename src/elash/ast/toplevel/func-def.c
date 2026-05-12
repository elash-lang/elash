#include <elash/ast/toplevel.h>

ElAstTopLevelNode* el_ast_new_func_def(ElDynArena* arena, ElSourceSpan span, ElAstFuncSignature sig, ElAstBlockStmtNode* block) {
    ElAstTopLevelNode* node = EL_DYNARENA_NEW(arena, ElAstTopLevelNode);
    *node = (ElAstTopLevelNode) {
        .type = EL_AST_TOPLVL_FUNC_DEF,
        .span = span,
        .next = NULL,
        .as.func_def = {
            .sig = sig,
            .block = block,
        }
    };
    return node;
}
