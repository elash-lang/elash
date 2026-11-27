#include <elash/mir/dump/instr.h>
#include <elash/mir/dump/value.h>
#include <elash/sema/unary-op.h>
#include <elash/sema/bin-op.h>
#include <elash/defs/sv.h>

void el_mir_dump_instr(const ElMirInstr* instr, usize indent, ElWriter out) {
    if (instr == NULL) return;

    for (usize i = 0; i < indent; ++i) el_write_char(out, ' ');

    if (instr->result != NULL) {
        el_mir_dump_value(instr->result, out);
        el_swrite(out, " = ");
    }

    switch (instr->kind) {
    case EL_MIR_INSTR_BIN: {
        ElStringView op = el_bin_op_to_string(instr->as.bin.op);
        el_writef(out, ""EL_SV_FMT" ", EL_SV_FARG(op));
        el_mir_dump_value(instr->as.bin.lhs, out);
        el_swrite(out, ", ");
        el_mir_dump_value(instr->as.bin.rhs, out);
        break;
    }
    case EL_MIR_INSTR_UNARY: {
        ElStringView op = el_unary_op_to_string(instr->as.unary.op);
        el_writef(out, ""EL_SV_FMT" ", EL_SV_FARG(op));
        el_mir_dump_value(instr->as.unary.operand, out);
        break;
    }
    case EL_MIR_INSTR_RET:
        el_swrite(out, "ret ");
        if (instr->as.return_.value != NULL) {
            el_mir_dump_value(instr->as.return_.value, out);
        } else {
            el_swrite(out, "void");
        }
        break;
    case EL_MIR_INSTR_CALL:
        el_swrite(out, "call ");
        el_mir_dump_value(instr->as.call.callee, out);
        el_write_char(out, '(');
        for (uint32_t i = 0; i < instr->as.call.arg_count; ++i) {
            if (i > 0) el_swrite(out, ", ");
            el_mir_dump_value(instr->as.call.args[i], out);
        }
        el_write_char(out, ')');
        break;
    case EL_MIR_INSTR_ALLOCA:
        el_swrite(out, "alloca ");
        el_mir_dump_type(instr->as.alloca.type, out);
        break;
    case EL_MIR_INSTR_LOAD:
        el_swrite(out, "load ");
        el_mir_dump_value(instr->as.load.ptr, out);
        break;
    case EL_MIR_INSTR_STORE:
        el_swrite(out, "store ");
        el_mir_dump_value(instr->as.store.value, out);
        el_swrite(out, ", ");
        el_mir_dump_value(instr->as.store.ptr, out);
        break;
    case EL_MIR_INSTR_JMP:
        el_writef(out, "jmp @%u", instr->as.jmp.target_id);
        break;
    case EL_MIR_INSTR_JMPIF:
        el_swrite(out, "jmpif ");
        el_mir_dump_value(instr->as.jmpif.cond, out);
        el_writef(out, ", @%u, @%u", instr->as.jmpif.then_id, instr->as.jmpif.else_id);
        break;
    case EL_MIR_INSTR_INTCAST:
        el_swrite(out, "intcast ");
        el_mir_dump_value(instr->as.intcast.operand, out);
        break;
    case EL_MIR_INSTR_FPCAST:
        el_swrite(out, "fpcast ");
        el_mir_dump_value(instr->as.fpcast.operand, out);
        break;
    case EL_MIR_INSTR_BITCAST:
        el_swrite(out, "bitcast ");
        el_mir_dump_value(instr->as.intcast.operand, out);
        break;
    case EL_MIR_INSTR_GEP:
        el_swrite(out, "gep ");
        el_mir_dump_value(instr->as.gep.ptr, out);
        el_swrite(out, ", ");
        el_mir_dump_value(instr->as.gep.index, out);
        break;
    case EL_MIR_INSTR_GFP:
        el_swrite(out, "gfp ");
        el_mir_dump_value(instr->as.gfp.ptr, out);
        el_writef(out, ", %zu", instr->as.gfp.index);
        break;
    case EL_MIR_INSTR_UNREACHABLE:
        el_swrite(out, "unreachable");
        break;
    }

    el_write_char(out, '\n');
}
