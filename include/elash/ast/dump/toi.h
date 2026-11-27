#pragma once

#include <elash/defs/int-types.h>
#include <elash/util/writer.h>

typedef struct ElAstToI ElAstToI;

void el_ast_dump_type_or_init(ElAstToI* node, usize indent, ElWriter out);
