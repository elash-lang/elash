#pragma once

#include <elash/mir/value.h>

#include <elash/sema/bin-op.h>
#include <elash/util/dynarena.h>

typedef struct ElMirInstr ElMirInstr;

typedef struct ElMirBinInstr {
    ElBinOp op;
    ElMirValue* lhs;
    ElMirValue* rhs;
} ElMirBinInstr;

ElMirInstr* el_mir_new_bin_instr(ElDynArena* arena, ElMirValue* result, ElBinOp op, ElMirValue* lhs, ElMirValue* rhs);
