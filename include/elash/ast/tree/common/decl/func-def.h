#pragma once

#include <elash/ast/tree/common/func.h>
#include <elash/ast/tree/common/block.h>

typedef struct ElAstDecl ElAstDecl;

typedef struct ElAstFuncDef {
    ElAstFuncSignature sig;
    ElAstBlockStmt*    block;
} ElAstFuncDef;

ElAstDecl* el_ast_new_func_def(ElDynArena* arena, ElSourceSpan span, ElAstFuncSignature sig, ElAstBlockStmt* block);
