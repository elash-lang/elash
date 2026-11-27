#pragma once

#include <elash/defs/int-types.h>
#include <elash/util/writer.h>

typedef struct ElAstStmt ElAstStmt;
void el_ast_dump_stmt(ElAstStmt* node, usize indent, ElWriter* out);
