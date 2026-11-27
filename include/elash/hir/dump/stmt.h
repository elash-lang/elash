#pragma once

#include <elash/defs/int-types.h>
#include <elash/util/writer.h>

typedef struct ElHirStmt ElHirStmt;

void el_hir_dump_stmt(ElHirStmt* node, usize indent, ElWriter out);
