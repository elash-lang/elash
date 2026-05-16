#pragma once

#include <elash/util/dynarena.h>

typedef struct ElHirStmtNode ElHirStmtNode;
typedef struct ElHirBreakStmtNode {
} ElHirBreakStmtNode;

ElHirStmtNode* el_hir_new_break_stmt(ElDynArena* arena);
