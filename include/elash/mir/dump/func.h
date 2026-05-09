#pragma once

#include <elash/mir/func.h>
#include <elash/defs/int-types.h>
#include <stdio.h>

void el_mir_dump_func(const ElMirFunc* func, usize indent, FILE* out);
