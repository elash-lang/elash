#include <elash/mir/instr/unreachable.h>
#include <elash/mir/instr.h>

ElMirInstr* el_mir_new_unreachable_instr(ElDynArena* arena) {
    ElMirInstr* instr = EL_DYNARENA_NEW(arena, ElMirInstr);
    instr->kind = EL_MIR_INSTR_UNREACHABLE;
    instr->result = NULL;
    return instr;
}
