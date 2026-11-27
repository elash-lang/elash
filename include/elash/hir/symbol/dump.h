#pragma once

#include <elash/util/writer.h>

typedef struct ElHirSymbol ElHirSymbol;

void el_dump_symbol(ElHirSymbol* symbol, ElWriter* out);
