#include <elash/mir/instr/bitcast.h>
#include <elash/mir/instr.h>

ElMirInstr* el_mir_new_bitcast_instr(ElDynArena* arena, ElMirValue* result, ElMirValue* operand) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElMirInstr, {
        .kind = EL_MIR_INSTR_BITCAST,
        .result = result,
        .as.bitcast.operand = operand,
    });
}
