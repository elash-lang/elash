#include <elash/hir/dump/expr.h>
#include <elash/hir/symbol/dump.h>
#include <elash/hir/dump/indent.h>
#include <elash/util/assert.h>

#include <elash/hir/tree/expr.h>

#include <elash/sema/bin-op.h>
#include <elash/sema/unary-op.h>

#include <inttypes.h>
#include <elash/defs/sv.h>

// TODO: split this shit into smaller helper functions
//       clang-tidy: Function 'el_hir_dump_expr' has cognitive complexity of 33 (threshold 25)
void el_hir_dump_expr(ElHirExpr* node, usize indent, FILE* out) {
    el_hir_dump_print_indent(indent, out);
    fputs("(", out);

    switch (node->kind) {
    case EL_HIR_EXPR_BINARY: {
        ElStringView op = el_sema_bin_op_to_string(node->as.binary.op);
        el_hir_dump_expr(node->as.binary.left, 0, out);
        fprintf(out, " "EL_SV_FMT" ", EL_SV_FARG(op));
        el_hir_dump_expr(node->as.binary.right, 0, out);
        break;
    }

    case EL_HIR_EXPR_UNARY: {
        ElStringView op = el_sema_unary_op_to_string(node->as.unary.op);
        if (!el_sema_unary_op_is_post(node->as.unary.op)) el_sv_print(op, out);
        el_hir_dump_expr(node->as.unary.operand, 0, out);
        if (el_sema_unary_op_is_post(node->as.unary.op))  el_sv_print(op, out);
        break;
    }

    case EL_HIR_EXPR_CONST:
        if (node->type->kind == EL_HIR_TYPE_PRIM) {
            switch (node->type->as.prim.kind) {
            case EL_PRIMTYPE_INT:   fprintf(out, "%"PRId64, node->as.constant.as.int_);        break;
            case EL_PRIMTYPE_CHAR:  fprintf(out, "'%c'", node->as.constant.as.char_);          break;
            case EL_PRIMTYPE_BOOL:  fputs(node->as.constant.as.bool_ ? "true" : "false", out); break;
            case EL_PRIMTYPE_VOID:  EL_UNREACHABLE("void literal");                            break;
            }
        } else {
            fputs("<unhandled>", out);
        }
        break;

    case EL_HIR_EXPR_SYMBOL:
        el_sema_dump_symbol(node->as.symbol, out);
        break;

    case EL_HIR_EXPR_CALL: {
        el_hir_dump_expr(node->as.call.callee, 0, out);
        fputs("(", out);
        for (usize i = 0; i < node->as.call.arg_count; ++i) {
            if (i > 0) fputs(", ", out);
            el_hir_dump_expr(node->as.call.args[i], 0, out);
        }
        fputs(")", out);
        break;
    }

    case EL_HIR_EXPR_ARRAYLIT:
        fputs("{", out);
        for (usize i = 0; i < node->as.array_lit.count; ++i) {
            if (i > 0) fputs(", ", out);
            el_hir_dump_expr(node->as.array_lit.values[i], 0, out);
        }
        fputs("}", out);
        break;

    case EL_HIR_EXPR_INTR:
        switch (node->as.intr.kind) {
            case EL_HIR_INTR_SLICE_LEN:  fputs("intr 'slice-len'", out);  break;
            case EL_HIR_INTR_SLICE_DATA: fputs("intr 'slice-data'", out); break;
            case EL_HIR_INTR_MAKE_SLICE: fputs("intr 'make-slice'", out); break;
        }
        fputs(" (", out);
        if (node->as.intr.kind == EL_HIR_INTR_MAKE_SLICE) {
            el_hir_dump_expr(node->as.intr.params.rwslice, 0, out);
            fputs(", ", out);
            el_hir_dump_expr(node->as.intr.params.len, 0, out);
        } else {
            el_hir_dump_expr(node->as.intr.params.slice, 0, out);
        }
        fputs(")", out);
        break;

    case EL_HIR_EXPR_CAST:
        // the type is already dumped after the switch so this should be enough
        fputs("cast(", out);
        el_hir_dump_expr(node->as.cast.expr, 0, out);
        fputs(")", out);
        break;

    case EL_HIR_EXPR_UNTYPEDLIT:
        switch (node->as.untyped_lit.kind) {
            case EL_HIR_UNTYPED_INT:  fprintf(out, "%"PRId64, node->as.untyped_lit.of.int_); break;
            case EL_HIR_UNTYPED_CHAR: fprintf(out, "'%c'", node->as.untyped_lit.of.char_);   break;
            case EL_HIR_UNTYPED_BOOL: fputs(node->as.untyped_lit.of.bool_ ? "true" : "false", out); break;
        }
        break;

    case EL_HIR_EXPR_MEMBER:
        el_hir_dump_expr(node->as.member.expr, 0, out);
        fprintf(out, "." EL_SV_FMT, EL_SV_FARG(node->as.member.name));
        break;

    case EL_HIR_EXPR_TMEMBER:
        el_hir_dump_expr(node->as.tmember.expr, 0, out);
        fprintf(out, ".%"PRIu64, (uint64_t)node->as.tmember.index);
        break;
    }

    fputs(" : ", out);
    if (node->type != NULL) {
        el_sema_dump_type(node->type, out);
    } else {
        fputs("untyped", out);
    }
    fputs(")", out);
}
