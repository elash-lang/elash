#pragma once

#include <elash/mir/value.h>

#include <elash/sema/unary-op.h>
#include <elash/util/dynarena.h>

typedef struct ElMirInstr ElMirInstr;

typedef struct ElMirUnaryInstr {
    ElSemaUnaryOp op;
    ElMirValue* operand;
} ElMirUnaryInstr;

ElMirInstr* el_mir_new_unary_instr(ElDynArena* arena, ElMirValue* result, ElSemaUnaryOp op, ElMirValue* operand);
