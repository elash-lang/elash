#pragma once

#include <elash/defs/int-types.h>
#include <elash/util/writer.h>

typedef struct ElAstModule ElAstModule;
void el_ast_dump_module(ElAstModule* mod, usize indent, ElWriter* out);
