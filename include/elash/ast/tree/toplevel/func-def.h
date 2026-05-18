#pragma once

#include <elash/ast/tree/common/func.h>

typedef struct ElAstFuncDef {
    ElAstFuncSignature sig;
    ElAstBlockStmt*    block;
} ElAstFuncDef;

ElAstTopLevel* el_ast_new_func_def(ElDynArena* arena, ElSourceSpan span, ElAstFuncSignature sig, ElAstBlockStmt* block);
