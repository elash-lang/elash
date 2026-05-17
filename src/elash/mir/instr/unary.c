#include <elash/mir/instr.h>

ElMirInstr* el_mir_new_unary_instr(ElDynArena* arena, ElMirValue* result, ElSemaUnaryOp op, ElMirValue* operand) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElMirInstr, {
        .kind = EL_MIR_INSTR_UNARY,
        .result = result,
        .as.unary = {
            .op = op,
            .operand = operand,
        },
    });
}
