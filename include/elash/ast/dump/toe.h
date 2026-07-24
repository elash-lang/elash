#pragma once

#include <elash/defs/int-types.h>
#include <stdio.h>

typedef struct ElAstTypeOrExpr ElAstTypeOrExpr;

void el_ast_dump_type_or_expr(ElAstTypeOrExpr* node, usize indent, FILE* out);
