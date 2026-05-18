#pragma once

#include <elash/defs/int-types.h>
#include <stdio.h>

typedef struct ElAstTopLevel ElAstTopLevel;
void el_ast_dump_toplevel(ElAstTopLevel* node, usize indent, FILE* out);
