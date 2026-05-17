#include <elash/mir/instr.h>

ElMirInstr* el_mir_new_store_instr(ElDynArena* arena, ElMirValue* ptr, ElMirValue* value) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElMirInstr, {
        .kind = EL_MIR_INSTR_STORE,
        .result = NULL,
        .as.store = {
            .ptr = ptr,
            .value = value,
        },
    });
}
