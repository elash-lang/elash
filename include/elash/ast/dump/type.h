#pragma once

#include <elash/ast/tree/common/type.h>
#include <elash/defs/int-types.h>
#include <stdio.h>

void el_ast_dump_type(ElAstTypeNode* node, usize indent, FILE* out);
