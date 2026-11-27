#pragma once

#include <elash/defs/int-types.h>
#include <elash/util/writer.h>

typedef struct ElAstExpr ElAstExpr;
void el_ast_dump_expr(ElAstExpr* node, usize indent, ElWriter out);
