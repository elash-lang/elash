#pragma once

#include <elash/util/dynarena.h>

typedef struct ElHirStmtNode ElHirStmtNode;
typedef struct ElHirContinueStmtNode {
} ElHirContinueStmtNode;

ElHirStmtNode* el_hir_new_continue_stmt(ElDynArena* arena);

