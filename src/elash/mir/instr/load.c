#include <elash/mir/instr.h>

ElMirInstr* el_mir_new_load_instr(ElDynArena* arena, ElMirValue* result, ElMirValue* ptr) {
    ElMirInstr* instr = EL_DYNARENA_NEW(arena, ElMirInstr);
    *instr = (ElMirInstr) {
        .kind = EL_MIR_INSTR_LOAD,
        .result = result,
        .as.load = {
            .ptr = ptr,
        },
    };
    return instr;
}
