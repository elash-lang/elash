#pragma once

#include <elash/util/dynarena.h>

typedef struct ElHirStmt ElHirStmt;

typedef struct ElHirBlockStmt {
    ElHirStmt* stmts;
} ElHirBlockStmt;

ElHirStmt* el_hir_new_block_stmt(ElDynArena* arena, ElHirStmt* stmts);
