#pragma once

#include <elash/defs/int-types.h>
#include <elash/util/writer.h>

typedef struct ElHirExpr ElHirExpr;

void el_hir_dump_expr(ElHirExpr* node, usize indent, ElWriter* out);
