#pragma once

#include <elash/hir/tree/decl.h>
#include <elash/defs/int-types.h>
#include <elash/util/writer.h>

void el_hir_dump_decl(ElHirDecl* node, usize indent, ElWriter out);
