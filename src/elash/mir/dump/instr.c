#include <elash/mir/dump/instr.h>
#include <elash/mir/dump/value.h>
#include <elash/sema/expr/bin-op.h>
#include <elash/sema/expr/unary-op.h>
#include <elash/defs/sv.h>

void el_mir_dump_instr(const ElMirInstr* instr, usize indent, FILE* out) {
    if (!instr) return;

    for (usize i = 0; i < indent; ++i) fputs(" ", out);

    if (instr->result) {
        el_mir_dump_value(instr->result, out);
        fputs(" = ", out);
    }

    switch (instr->kind) {
    case EL_MIR_INSTR_BIN: {
        ElStringView op = el_sema_bin_op_to_string(instr->as.bin.op);
        fprintf(out, ""EL_SV_FMT" ", EL_SV_FARG(op));
        el_mir_dump_value(instr->as.bin.lhs, out);
        fputs(", ", out);
        el_mir_dump_value(instr->as.bin.rhs, out);
        break;
    }
    case EL_MIR_INSTR_UNARY: {
        ElStringView op = el_sema_unary_op_to_string(instr->as.unary.op);
        fprintf(out, ""EL_SV_FMT" ", EL_SV_FARG(op));
        el_mir_dump_value(instr->as.unary.operand, out);
        break;
    }
    case EL_MIR_INSTR_RET:
        fputs("ret ", out);
        if (instr->as.return_.value) {
            el_mir_dump_value(instr->as.return_.value, out);
        } else {
            fputs("void", out);
        }
        break;
    case EL_MIR_INSTR_CALL: {
        fputs("call ", out);
        el_mir_dump_value(instr->as.call.callee, out);
        fputs("(", out);
        for (uint32_t i = 0; i < instr->as.call.arg_count; ++i) {
            if (i > 0) fputs(", ", out);
            el_mir_dump_value(instr->as.call.args[i], out);
        }
        fputs(")", out);
        break;
    }
    case EL_MIR_INSTR_ALLOCA: {
        fputs("alloca ", out);
        el_sema_dump_type(instr->as.alloca.type, out);
        break;
    }
    case EL_MIR_INSTR_LOAD: {
        fputs("load ", out);
        el_mir_dump_value(instr->as.load.ptr, out);
        break;
    }
    case EL_MIR_INSTR_STORE: {
        fputs("store ", out);
        el_mir_dump_value(instr->as.store.value, out);
        fputs(", ", out);
        el_mir_dump_value(instr->as.store.ptr, out);
        break;
    }
    case EL_MIR_INSTR_JMP: {
        fprintf(out, "jmp @%u", instr->as.jmp.target_id);
        break;
    }
    case EL_MIR_INSTR_JMPIF: {
        fputs("jmpif ", out);
        el_mir_dump_value(instr->as.jmpif.cond, out);
        fprintf(out, ", @%u, @%u", instr->as.jmpif.then_id, instr->as.jmpif.else_id);
        break;
    }
    case EL_MIR_INSTR_UNREACHABLE:
        fputs("unreachable", out);
        break;
    }

    fputs("\n", out);
}
