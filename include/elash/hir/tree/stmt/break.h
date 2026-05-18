#pragma once

#include <elash/util/dynarena.h>

typedef struct ElHirStmt ElHirStmt;
typedef struct ElHirBreakStmt {
} ElHirBreakStmt;

ElHirStmt* el_hir_new_break_stmt(ElDynArena* arena);
