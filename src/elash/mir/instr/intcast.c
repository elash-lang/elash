#include <elash/mir/instr/intcast.h>
#include <elash/mir/instr.h>

ElMirInstr* el_mir_new_intcast_instr(ElDynArena* arena, ElMirValue* result, ElMirValue* operand) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElMirInstr, {
        .kind = EL_MIR_INSTR_INTCAST,
        .result = result,
        .as.intcast.operand = operand,
    });
}
