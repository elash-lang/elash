#pragma once

#include <elash/ast/tree/common/func.h>

typedef struct ElAstFuncDecl {
    ElAstFuncSignature sig;
} ElAstFuncDecl;

ElAstTopLevel* el_ast_new_func_decl(ElDynArena* arena, ElSourceSpan span, ElAstFuncSignature sig);
