#include <elash/mir/instr.h>

ElMirInstr* el_mir_new_gfp_instr(ElDynArena* arena, ElMirValue* result, ElMirValue* ptr, usize index) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElMirInstr, {
        .kind = EL_MIR_INSTR_GFP,
        .result = result,
        .as.gfp = {
            .ptr = ptr,
            .index = index,
        },
    });
}
