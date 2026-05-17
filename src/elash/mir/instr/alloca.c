#include <elash/mir/instr.h>

ElMirInstr* el_mir_new_alloca_instr(ElDynArena* arena, ElMirValue* result, ElType* type) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElMirInstr, {
        .kind = EL_MIR_INSTR_ALLOCA,
        .result = result,
        .as.alloca = {
            .type = type,
        },
    });
}
