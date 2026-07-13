#pragma once

#include <elash/util/dynarena.h>
#include <elash/mir/value.h>

#include "instr/binary.h"
#include "instr/unary.h"
#include "instr/intcast.h"
#include "instr/bitcast.h"
#include "instr/return.h"
#include "instr/call.h"
#include "instr/alloca.h"
#include "instr/load.h"
#include "instr/store.h"
#include "instr/jump.h"
#include "instr/gep.h"
#include "instr/unreachable.h"

typedef enum ElMirInstrKind {
    EL_MIR_INSTR_BIN,
    EL_MIR_INSTR_UNARY,
    EL_MIR_INSTR_INTCAST,
    EL_MIR_INSTR_BITCAST,
    EL_MIR_INSTR_RET,
    EL_MIR_INSTR_CALL,
    EL_MIR_INSTR_ALLOCA,
    EL_MIR_INSTR_LOAD,
    EL_MIR_INSTR_STORE,
    EL_MIR_INSTR_GEP,
    EL_MIR_INSTR_JMP,
    EL_MIR_INSTR_JMPIF,
    EL_MIR_INSTR_UNREACHABLE,
} ElMirInstrKind;

typedef struct ElMirInstr ElMirInstr;

struct ElMirInstr {
    ElMirInstrKind kind;
    ElMirValue* result;

    union {
        ElMirBinInstr     bin;
        ElMirUnaryInstr   unary;
        ElMirIntCastInstr intcast;
        ElMirBitCastInstr bitcast;
        ElMirRetInstr     return_;
        ElMirCallInstr    call;
        ElMirAllocaInstr  alloca;
        ElMirLoadInstr    load;
        ElMirStoreInstr   store;
        ElMirGepInstr     gep;
        ElMirJmpInstr     jmp;
        ElMirJmpIfInstr   jmpif;
    } as;
};

static inline bool el_mir_instr_is_terminator(ElMirInstr* instr) {
    return instr->kind == EL_MIR_INSTR_RET ||
           instr->kind == EL_MIR_INSTR_JMP ||
           instr->kind == EL_MIR_INSTR_JMPIF ||
           instr->kind == EL_MIR_INSTR_UNREACHABLE;
}
