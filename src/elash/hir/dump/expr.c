#include <elash/hir/dump/expr.h>
#include <elash/hir/symbol/dump.h>
#include <elash/hir/dump/indent.h>
#include <elash/util/assert.h>

#include <elash/hir/tree/expr.h>

#include <elash/sema/bin-op.h>
#include <elash/sema/unary-op.h>
#include <elash/util/int128.h>

#include <inttypes.h>
#include <elash/defs/sv.h>

void dump_constant(ElHirExpr* node, ElHirType* type, ElWriter out) {
    type = el_hir_type_canonical(type);
    if (type->kind == EL_HIR_TYPE_PRIM) {
        switch (type->as.prim.kind) {
        case EL_HIR_PRIMTYPE_INT:   el_writef(out, "%" PRId64, (int64_t)el_i128_lo(node->as.constant.as.int_)); return;
        case EL_HIR_PRIMTYPE_FLOAT: el_writef(out, "%lf", node->as.constant.as.float_);          return;
        case EL_HIR_PRIMTYPE_BOOL:  el_writef(out, "%s", node->as.constant.as.bool_ ? "true" : "false"); return;
        case EL_HIR_PRIMTYPE_VOID:  EL_UNREACHABLE("void literal");                            return;
        }
        EL_UNREACHABLE_ENUM_VAL(ElHirPrimTypeKind, type->as.prim.kind);
    } else if (type->kind == EL_HIR_TYPE_DISTINCT) {
        ElHirType* orig = el_hir_type_unwrap(type);
        bool is_char = el_sv_eql(type->as.distinct.name, EL_SV("char"))
            && orig->kind == EL_HIR_TYPE_PRIM
            && orig->as.prim.as.integral.width == EL_HIR_IWIDTH_8
            && orig->as.prim.as.integral.is_signed == false;

        if (is_char) {
            el_writef(out, "%c", (char)el_i128_lo(node->as.constant.as.int_));
            return;
        } else {
            return dump_constant(node, orig, out);
        }
    }

    EL_UNREACHABLE("unexpected literal type");
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity): the logic is flat, so it's ok.
void el_hir_dump_expr(ElHirExpr* node, usize indent, ElWriter out) {
    el_hir_dump_print_indent(indent, out);
    el_write_char(out, '(');

    switch (node->kind) {
    case EL_HIR_EXPR_BINARY: {
        ElStringView op = el_bin_op_to_string(node->as.binary.op);
        el_hir_dump_expr(node->as.binary.left, 0, out);
        el_writef(out, " "EL_SV_FMT" ", EL_SV_FARG(op));
        el_hir_dump_expr(node->as.binary.right, 0, out);
        break;
    }

    case EL_HIR_EXPR_UNARY: {
        ElStringView op = el_unary_op_to_string(node->as.unary.op);
        if (!el_unary_op_is_post(node->as.unary.op)) el_write(out, op);
        el_hir_dump_expr(node->as.unary.operand, 0, out);
        if (el_unary_op_is_post(node->as.unary.op))  el_write(out, op);
        break;
    }

    case EL_HIR_EXPR_CONST:
        dump_constant(node, node->type, out);
        break;

    case EL_HIR_EXPR_STRCONST:
        el_writef(out, "\"" EL_SV_FMT "\"", EL_SV_FARG(node->as.strconst.chars));
        break;

    case EL_HIR_EXPR_SYMBOL:
        el_dump_symbol(node->as.symbol, out);
        break;

    case EL_HIR_EXPR_CALL: {
        el_hir_dump_expr(node->as.call.callee, 0, out);
        el_write_char(out, '(');
        for (usize i = 0; i < node->as.call.arg_count; ++i) {
            if (i > 0) el_swrite(out, ", ");
            el_hir_dump_expr(node->as.call.args[i], 0, out);
        }
        el_write_char(out, ')');
        break;
    }

    case EL_HIR_EXPR_AGGINIT:
        el_write_char(out, '{');
        for (usize i = 0; i < node->as.agginit.count; ++i) {
            if (i > 0) el_swrite(out, ", ");
            el_hir_dump_expr(node->as.agginit.values[i], 0, out);
        }
        el_write_char(out, '}');
        break;

    case EL_HIR_EXPR_INTR:
        switch (node->as.intr.kind) {
        case EL_HIR_INTR_SLICE_LEN:  el_swrite(out, "intr 'slice-len'");  break;
        case EL_HIR_INTR_SLICE_DATA: el_swrite(out, "intr 'slice-data'"); break;
        case EL_HIR_INTR_MAKE_SLICE: el_swrite(out, "intr 'make-slice'"); break;
        case EL_HIR_INTR_EXCHANGE:   el_swrite(out, "intr 'exchange'");   break;
        case EL_HIR_INTR_NULL_OPT:   el_swrite(out, "intr 'null-opt'");   break;
        case EL_HIR_INTR_SOME_OPT:   el_swrite(out, "intr 'some-opt'");   break;
        }

        el_swrite(out, " (");
        if (node->as.intr.kind == EL_HIR_INTR_MAKE_SLICE) {
            el_hir_dump_expr(node->as.intr.params.rwslice, 0, out);
            el_swrite(out, ", ");
            el_hir_dump_expr(node->as.intr.params.len, 0, out);
        } else if (node->as.intr.kind == EL_HIR_INTR_EXCHANGE) {
            el_hir_dump_expr(node->as.intr.params.exchange.dest, 0, out);
            el_swrite(out, ", ");
            el_hir_dump_expr(node->as.intr.params.exchange.value, 0, out);
        } else if (node->as.intr.kind == EL_HIR_INTR_SOME_OPT) {
            el_hir_dump_expr(node->as.intr.params.value, 0, out);
        } else if (node->as.intr.kind != EL_HIR_INTR_NULL_OPT) {
            el_hir_dump_expr(node->as.intr.params.slice, 0, out);
        }
        el_write_char(out, ')');
        break;

    case EL_HIR_EXPR_CAST:
        // the type is already dumped after the switch so this should be enough
        el_swrite(out, "cast(");
        el_hir_dump_expr(node->as.cast.expr, 0, out);
        el_write_char(out, ')');
        break;

    case EL_HIR_EXPR_LITERAL:
        switch (node->as.literal.kind) {
        case EL_HIR_LITERAL_INT:
            // NOLINTBEGIN(readability-magic-numbers): We're no strangers to love
            el_write(out, el_i128_to_string(node->as.literal.of.int_, 10, (char[42]){}));
            break;
            // NOLINTEND(readability-magic-numbers)
        case EL_HIR_LITERAL_FLOAT:  el_writef(out, "%lf", node->as.literal.of.float_);          break;
        case EL_HIR_LITERAL_CHAR:   el_writef(out, "'%c'", node->as.literal.of.char_);          break;
        case EL_HIR_LITERAL_BOOL:   el_writef(out, "%s", node->as.literal.of.bool_ ? "true" : "false"); break;
        case EL_HIR_LITERAL_STRING: el_write(out, node->as.literal.of.str_);                     break;
        case EL_HIR_LITERAL_NULL:   el_swrite(out, "null");                                    break;
        }
        break;

    case EL_HIR_EXPR_MEMBER:
        el_hir_dump_expr(node->as.member.expr, 0, out);
        el_writef(out, "." EL_SV_FMT, EL_SV_FARG(node->as.member.name));
        break;

    case EL_HIR_EXPR_TMEMBER:
        el_hir_dump_expr(node->as.tmember.expr, 0, out);
        el_writef(out, ".%"PRIu64, (uint64_t)node->as.tmember.index);
        break;
    }

    el_swrite(out, " : ");
    if (node->type != NULL) {
        el_hir_dump_type(node->type, out);
    } else {
        el_swrite(out, "untyped");
    }
    el_write_char(out, ')');
}
