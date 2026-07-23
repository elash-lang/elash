#pragma once

#include <elash/util/dynarena.h>
#include <elash/srcdoc/span.h>

typedef struct ElHirStmt ElHirStmt;

typedef struct ElHirBlockStmt {
    ElHirStmt* stmts;
} ElHirBlockStmt;

ElHirStmt* el_hir_new_block_stmt(ElDynArena* arena, ElSourceSpan span, ElHirStmt* stmts);
