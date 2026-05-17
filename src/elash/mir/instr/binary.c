#include <elash/mir/instr.h>

ElMirInstr* el_mir_new_bin_instr(ElDynArena* arena, ElMirValue* result, ElSemaBinOp op, ElMirValue* lhs, ElMirValue* rhs) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElMirInstr, {
        .kind = EL_MIR_INSTR_BIN,
        .result = result,
        .as.bin = {
            .op = op,
            .lhs = lhs,
            .rhs = rhs,
        },
    });
}
