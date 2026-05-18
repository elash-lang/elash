#pragma once

#include <elash/defs/int-types.h>
#include <stdio.h>

typedef struct ElAstInit ElAstInit;
void el_ast_dump_init(ElAstInit* init, usize indent, FILE* out);
