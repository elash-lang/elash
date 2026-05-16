#pragma once

#include <elash/util/dynarena.h>
#include <elash/srcdoc/span.h>

typedef struct ElAstStmtNode ElAstStmtNode;
typedef struct ElAstContinueStmtNode {
} ElAstContinueStmtNode;

ElAstStmtNode* el_ast_new_continue_stmt(ElDynArena* arena, ElSourceSpan span);
