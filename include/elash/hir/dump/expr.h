#pragma once

#include <elash/defs/int-types.h>
#include <stdio.h>

typedef struct ElHirExpr ElHirExpr;

void el_hir_dump_expr(ElHirExpr* node, usize indent, FILE* out);
