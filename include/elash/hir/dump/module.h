#pragma once

#include <elash/defs/int-types.h>
#include <elash/util/writer.h>

typedef struct ElHirModule ElHirModule;

void el_hir_dump_module(ElHirModule* module, usize indent, ElWriter* out);
