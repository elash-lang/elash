#pragma once

#include <elash/mir/block.h>
#include <elash/defs/int-types.h>
#include <elash/util/writer.h>

void el_mir_dump_block(const ElMirBlock* block, usize indent, ElWriter* out);
