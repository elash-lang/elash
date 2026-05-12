#pragma once

#include <elash/ast/common/func.h>

typedef struct ElAstFuncDecl {
    ElAstFuncSignature  sig;
} ElAstFuncDecl;

ElAstTopLevelNode* el_ast_new_func_decl(ElDynArena* arena, ElSourceSpan span, ElAstFuncSignature sig);
