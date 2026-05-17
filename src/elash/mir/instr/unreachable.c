#include <elash/mir/instr/unreachable.h>
#include <elash/mir/instr.h>

ElMirInstr* el_mir_new_unreachable_instr(ElDynArena* arena) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElMirInstr, {
        .kind = EL_MIR_INSTR_UNREACHABLE, 
        .result = NULL, 
    });
}
