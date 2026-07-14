#include <elash/mir/instr.h>

ElMirInstr* el_mir_new_load_instr(ElDynArena* arena, ElMirValue* result, ElMirValue* ref) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElMirInstr, {
        .kind = EL_MIR_INSTR_LOAD,
        .result = result,
        .as.load = {
            .ref = ref,
        },
    });
}
