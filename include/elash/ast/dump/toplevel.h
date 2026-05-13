#pragma once

#include <elash/defs/int-types.h>
#include <stdio.h>

typedef struct ElAstTopLevelNode ElAstTopLevelNode;
void el_ast_dump_toplevel(ElAstTopLevelNode* node, usize indent, FILE* out);

