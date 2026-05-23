#pragma once

#include <elash/ast/tree/common/func.h>

typedef struct ElAstDecl ElAstDecl;

typedef struct ElAstFuncDecl {
    ElAstFuncSignature sig;
} ElAstFuncDecl;

ElAstDecl* el_ast_new_func_decl(ElDynArena* arena, ElSourceSpan span, ElAstFuncSignature sig);
