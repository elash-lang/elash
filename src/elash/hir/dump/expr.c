#include <elash/hir/dump/expr.h>
#include <elash/sema/symbol/dump.h>
#include <elash/hir/dump/indent.h>
#include <elash/util/assert.h>

#include <elash/hir/tree/expr.h>

#include <elash/sema/expr/bin-op.h>
#include <elash/sema/expr/unary-op.h>

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

    case EL_HIR_EXPR_LITERAL:
        if (node->type->kind == EL_TYPE_PRIM) {
            switch (node->type->as.prim.kind) {
            case EL_PRIMTYPE_INT:   fprintf(out, "%"PRId64, node->as.literal.as.int_);        break;
            case EL_PRIMTYPE_CHAR:  fprintf(out, "'%c'", node->as.literal.as.char_);          break;
            case EL_PRIMTYPE_BOOL:  fputs(node->as.literal.as.bool_ ? "true" : "false", out); break;
            case EL_PRIMTYPE_VOID:  EL_UNREACHABLE("void literal");                           break;
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

    case EL_HIR_EXPR_ARRAY_LITERAL:
        fputs("{", out);
        for (usize i = 0; i < node->as.array_lit.count; ++i) {
            if (i > 0) fputs(", ", out);
            el_hir_dump_expr(node->as.array_lit.values[i], 0, out);
        }
        fputs("}", out);
        break;
    }

    fputs(" : ", out);
    el_sema_dump_type(node->type, out);
    fputs(")", out);
}
