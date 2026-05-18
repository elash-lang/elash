#pragma once

#include <elash/defs/int-types.h>
#include <stdio.h>

typedef struct ElHirTopLevel ElHirTopLevel;

void el_hir_dump_toplevel(ElHirTopLevel* node, usize indent, FILE* out);
