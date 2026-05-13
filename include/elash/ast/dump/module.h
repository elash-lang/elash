#pragma once

#include <elash/defs/int-types.h>
#include <stdio.h>

typedef struct ElAstModuleNode ElAstModuleNode;
void el_ast_dump_module(ElAstModuleNode* mod, usize indent, FILE* out);

