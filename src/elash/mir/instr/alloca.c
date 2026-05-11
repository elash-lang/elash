#include <elash/mir/instr.h>

ElMirInstr* el_mir_new_alloca_instr(ElDynArena* arena, ElMirValue* result, ElType* type) {
    ElMirInstr* instr = EL_DYNARENA_NEW(arena, ElMirInstr);
    *instr = (ElMirInstr) {
        .kind = EL_MIR_INSTR_ALLOCA,
        .result = result,
        .as.alloca = {
            .type = type,
        },
    };
    return instr;
}
