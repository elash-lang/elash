#pragma once

#include <elash/defs/int-types.h>
#include <elash/util/writer.h>

typedef struct ElAstInit ElAstInit;
void el_ast_dump_init(ElAstInit* init, usize indent, ElWriter out);
