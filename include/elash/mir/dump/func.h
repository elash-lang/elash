#pragma once

#include <elash/mir/func.h>
#include <elash/defs/int-types.h>
#include <elash/util/writer.h>

void el_mir_dump_func(const ElMirFunc* func, usize indent, ElWriter out);
