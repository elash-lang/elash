#pragma once

#include <elash/util/dynarena.h>
#include <elash/mir/value.h>

#include "instr/binary.h"
#include "instr/unary.h"
#include "instr/return.h"
#include "instr/call.h"
#include "instr/alloca.h"
#include "instr/load.h"
#include "instr/store.h"
#include "instr/jump.h"

typedef enum ElMirInstrKind {
    EL_MIR_INSTR_BIN,
    EL_MIR_INSTR_UNARY,
    EL_MIR_INSTR_RET,
    EL_MIR_INSTR_CALL,
    EL_MIR_INSTR_ALLOCA,
    EL_MIR_INSTR_LOAD,
    EL_MIR_INSTR_STORE,
    EL_MIR_INSTR_JMP,
    EL_MIR_INSTR_JMPIF,
} ElMirInstrKind;

typedef struct ElMirInstr ElMirInstr;

struct ElMirInstr {
    ElMirInstrKind kind;
    ElMirValue* result; 
    
    union {
        ElMirBinInstr    bin;
        ElMirUnaryInstr  unary;
        ElMirRetInstr    return_;
        ElMirCallInstr   call;
        ElMirAllocaInstr alloca;
        ElMirLoadInstr   load;
        ElMirStoreInstr  store;
        ElMirJmpInstr    jmp;
        ElMirJmpIfInstr  jmpif;
    } as;
};
