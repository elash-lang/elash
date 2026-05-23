#include <elash/ast/tree/common/decl.h>

ElAstDecl* el_ast_new_func_def(ElDynArena* arena, ElSourceSpan span, ElAstFuncSignature sig, ElAstBlockStmt* block) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstDecl, {
        .type = EL_AST_DECL_FUNC_DEF,
        .span = span,
        .as.func_def = {
            .sig = sig,
            .block = block,
        }
    });
}
