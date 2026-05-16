#pragma once

#include <elash/util/dynarena.h>
#include <elash/srcdoc/span.h>

typedef struct ElAstStmtNode ElAstStmtNode;
typedef struct ElAstBreakStmtNode {
} ElAstBreakStmtNode;

ElAstStmtNode* el_ast_new_break_stmt(ElDynArena* arena, ElSourceSpan span);
