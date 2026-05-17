#pragma once

#include <elash/defs/int-types.h>
#include <stdio.h>

typedef struct ElAstInitializer ElAstInitializer;
void el_ast_dump_init(ElAstInitializer* init, usize indent, FILE* out);

