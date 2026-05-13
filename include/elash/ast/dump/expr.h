#pragma once

#include <elash/defs/int-types.h>
#include <stdio.h>

typedef struct ElAstExprNode ElAstExprNode;
void el_ast_dump_expr(ElAstExprNode* node, usize indent, FILE* out);

