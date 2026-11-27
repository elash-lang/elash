#pragma once

#include <elash/defs/int-types.h>
#include <elash/util/writer.h>

typedef struct ElAstUnr ElAstUnr;

void el_ast_dump_unr(ElAstUnr* node, usize indent, ElWriter out);
