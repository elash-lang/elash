#include <elash/mir/instr/jump.h>
#include <elash/mir/instr.h>

ElMirInstr* el_mir_new_jmp_instr(ElDynArena* arena, uint32_t target_id) {
    ElMirInstr* instr = EL_DYNARENA_NEW(arena, ElMirInstr);
    instr->kind = EL_MIR_INSTR_JMP;
    instr->result = NULL;
    instr->as.jmp.target_id = target_id;
    return instr;
}

ElMirInstr* el_mir_new_jmpif_instr(ElDynArena* arena, ElMirValue* cond, uint32_t then_id, uint32_t else_id) {
    ElMirInstr* instr = EL_DYNARENA_NEW(arena, ElMirInstr);
    instr->kind = EL_MIR_INSTR_JMPIF;
    instr->result = NULL;
    instr->as.jmpif.cond = cond;
    instr->as.jmpif.then_id = then_id;
    instr->as.jmpif.else_id = else_id;
    return instr;
}
