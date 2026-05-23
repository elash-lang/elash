#include <elash/ast/tree/common/decl.h>

ElAstDecl* el_ast_new_func_decl(ElDynArena* arena, ElSourceSpan span, ElAstFuncSignature sig) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstDecl, {
        .type = EL_AST_DECL_FUNC_DECL,
        .span = span,
        .as.func_decl = {
            .sig = sig,
        }
    });
}
