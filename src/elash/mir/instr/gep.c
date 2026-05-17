#include <elash/mir/instr.h>

ElMirInstr* el_mir_new_gep_instr(ElDynArena* arena, ElMirValue* result, ElMirValue* ptr, ElMirValue* index) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElMirInstr, {
        .kind = EL_MIR_INSTR_GEP,
        .result = result,
        .as.gep = {
            .ptr = ptr,
            .index = index,
        },
    });
}
