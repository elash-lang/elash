#pragma once

#include <elash/defs/int-types.h>
#include <elash/util/writer.h>

typedef struct ElAstToE ElAstToE;

void el_ast_dump_type_or_expr(ElAstToE* node, usize indent, ElWriter out);
