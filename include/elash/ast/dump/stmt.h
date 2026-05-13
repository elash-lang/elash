#pragma once

#include <elash/defs/int-types.h>
#include <stdio.h>

typedef struct ElAstStmtNode ElAstStmtNode;
void el_ast_dump_stmt(ElAstStmtNode* node, usize indent, FILE* out);

