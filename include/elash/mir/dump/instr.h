#pragma once

#include <elash/mir/instr.h>
#include <elash/defs/int-types.h>
#include <stdio.h>

void el_mir_dump_instr(const ElMirInstr* instr, usize indent, FILE* out);
