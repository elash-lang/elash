#pragma once

#include <elash/ast/tree/common/func.h>

typedef struct ElAstFuncDef {
    ElAstFuncSignature  sig;
    ElAstBlockStmtNode* block;
} ElAstFuncDef;

ElAstTopLevelNode* el_ast_new_func_def(ElDynArena* arena, ElSourceSpan span, ElAstFuncSignature sig, ElAstBlockStmtNode* block);
