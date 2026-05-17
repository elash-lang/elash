#include <elash/mir/instr.h>

ElMirInstr* el_mir_new_call_instr(ElDynArena* arena, ElMirValue* result, ElMirValue* callee, ElMirValue** args, uint32_t arg_count) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElMirInstr, {
        .kind = EL_MIR_INSTR_CALL,
        .result = result,
        .as.call = {
            .callee = callee,
            .args = args,
            .arg_count = arg_count,
        },
    });
}
