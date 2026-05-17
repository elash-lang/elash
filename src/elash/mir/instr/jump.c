#include <elash/mir/instr/jump.h>
#include <elash/mir/instr.h>

ElMirInstr* el_mir_new_jmp_instr(ElDynArena* arena, uint32_t target_id) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElMirInstr, {
        .kind = EL_MIR_INSTR_JMP,
        .result = NULL,
        .as.jmp.target_id = target_id,
    });
}

ElMirInstr* el_mir_new_jmpif_instr(ElDynArena* arena, ElMirValue* cond, uint32_t then_id, uint32_t else_id) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElMirInstr, {
        .kind = EL_MIR_INSTR_JMPIF,
        .result = NULL,
        .as.jmpif = {
            .cond = cond,
            .then_id = then_id,
            .else_id = else_id,
        },
    });
}
