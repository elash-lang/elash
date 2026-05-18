#pragma once

#include <elash/defs/int-types.h>
#include <stdio.h>

typedef struct ElAstExpr ElAstExpr;
void el_ast_dump_expr(ElAstExpr* node, usize indent, FILE* out);
