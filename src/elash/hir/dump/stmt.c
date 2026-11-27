#include <elash/hir/dump/stmt.h>
#include <elash/hir/dump/expr.h>
#include <elash/hir/dump/indent.h>
#include <elash/hir/dump/decl.h>

#include <elash/hir/type.h>
#include <elash/hir/tree/stmt.h>

#include <elash/util/assert.h>

void el_hir_dump_stmt(ElHirStmt* node, usize indent, ElWriter out) {
    switch (node->kind) {
    case EL_HIR_STMT_EXPR:
        el_hir_dump_expr(node->as.expr, indent, out);
        el_swrite(out, ";\n");
        return;

    case EL_HIR_STMT_RETURN:
        el_hir_dump_print_indent(indent, out);
        el_swrite(out, "return");
        if (node->as.return_.value != NULL) {
            el_write_char(out, ' ');
            el_hir_dump_expr(node->as.return_.value, 0, out);
        }
        el_swrite(out, ";\n");
        return;

    case EL_HIR_STMT_DECL:
        el_hir_dump_decl(node->as.decl, indent, out);
        return;

    case EL_HIR_STMT_ASSIGN:
        el_hir_dump_expr(node->as.assign.target, indent, out);
        el_swrite(out, " = ");
        el_hir_dump_expr(node->as.assign.value, 0, out);
        el_swrite(out, ";\n");
        return;
    case EL_HIR_STMT_CASSIGN:
        el_hir_dump_expr(node->as.cassign.target, indent, out);
        el_writef(out, " "EL_SV_FMT"= ", EL_SV_FARG(el_bin_op_to_string(node->as.cassign.op)));
        el_hir_dump_expr(node->as.cassign.value, 0, out);
        el_swrite(out, ";\n");
        return;

    case EL_HIR_STMT_IF:
        el_hir_dump_print_indent(indent, out);
        el_swrite(out, "if (");
        el_hir_dump_expr(node->as.if_.cond, 0, out);
        el_swrite(out, ")\n");
        el_hir_dump_stmt(node->as.if_.then, indent + 1, out);
        if (node->as.if_.else_ != NULL) {
            el_hir_dump_print_indent(indent, out);
            el_swrite(out, "else\n");
            el_hir_dump_stmt(node->as.if_.else_, indent + 1, out);
        }
        return;

    case EL_HIR_STMT_WHILE:
        el_hir_dump_print_indent(indent, out);
        el_swrite(out, "while (");
        el_hir_dump_expr(node->as.while_.cond, 0, out);
        el_swrite(out, ")\n");
        el_hir_dump_stmt(node->as.while_.body, indent + 1, out);
        return;

    case EL_HIR_STMT_BREAK:
        el_hir_dump_print_indent(indent, out);
        el_swrite(out, "break;");
        return;
    case EL_HIR_STMT_CONTINUE:
        el_hir_dump_print_indent(indent, out);
        el_swrite(out, "continue;");
        return;

    case EL_HIR_STMT_BLOCK:
        el_hir_dump_print_indent(indent, out);
        el_swrite(out, "{\n");
        for (ElHirStmt* curr = node->as.block.stmts; curr; curr = curr->next) {
            el_hir_dump_stmt(curr, indent + 1, out);
        }
        el_hir_dump_print_indent(indent, out);
        el_swrite(out, "}\n");
        return;
    }
    EL_UNREACHABLE_ENUM_VAL(ElHirStmtKind, node->kind);
}
