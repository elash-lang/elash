#pragma once

#include <elash/defs/int-types.h>
#include <stdio.h>

typedef struct ElHirStmt ElHirStmt;

void el_hir_dump_stmt(ElHirStmt* node, usize indent, FILE* out);
