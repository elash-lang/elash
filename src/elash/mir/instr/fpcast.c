#include <elash/mir/instr/fpcast.h>
#include <elash/mir/instr.h>

ElMirInstr* el_mir_new_fpcast_instr(ElDynArena* arena, ElMirValue* result, ElMirValue* operand) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElMirInstr, {
        .kind = EL_MIR_INSTR_FPCAST,
        .result = result,
        .as.fpcast.operand = operand,
    });
}
