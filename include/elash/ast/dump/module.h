#pragma once

#include <elash/defs/int-types.h>
#include <stdio.h>

typedef struct ElAstModule ElAstModule;
void el_ast_dump_module(ElAstModule* mod, usize indent, FILE* out);
