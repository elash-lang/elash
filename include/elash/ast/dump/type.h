#pragma once

#include <elash/defs/int-types.h>
#include <elash/ast/tree/type.h>
#include <stdio.h>

void el_ast_dump_type(ElAstType* node, usize indent, FILE* out);
