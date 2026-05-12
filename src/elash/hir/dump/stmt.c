#include <elash/hir/dump/stmt.h>
#include <elash/hir/dump/expr.h>
#include <elash/hir/dump/indent.h>

#include <elash/sema/type.h>
#include <elash/hir/tree/stmt.h>

#include <elash/util/assert.h>

void el_hir_dump_stmt(ElHirStmtNode* node, usize indent, FILE* out) {
    if (!node) return;

    for (ElHirStmtNode* curr = node; curr; curr = curr->next) {
        switch (curr->kind) {
        case EL_HIR_STMT_EXPR:
            el_hir_dump_expr(curr->as.expr, indent, out);
            fputs(";\n", out);
            continue;

        case EL_HIR_STMT_RETURN:
            el_hir_dump_print_indent(indent, out);
            fputs("return", out);
            if (curr->as.return_.value) {
                fputs(" ", out);
                el_hir_dump_expr(curr->as.return_.value, 0, out);
            }
            fputs(";\n", out);
            continue;

        case EL_HIR_STMT_VAR_DEF: {
            ElHirVarDefStmtNode* var_def = &curr->as.var_def;
            el_hir_dump_print_indent(indent, out);
            el_sema_dump_type(var_def->var->as.var.type, out);
            fputc(' ', out);
            el_sv_print(var_def->var->name, out);
            if (var_def->init != NULL) {
                fputs(" = ", out);
                el_hir_dump_expr(var_def->init, 0, out);
            }
            fputs(";\n", out);
            continue;
        }

        case EL_HIR_STMT_IF:
            el_hir_dump_print_indent(indent, out);
            fputs("if (", out);
            el_hir_dump_expr(node->as.if_.cond, 0, out);
            fputs(")\n", out);
            el_hir_dump_stmt(node->as.if_.then, indent + 1, out);
            if (node->as.if_.else_ != NULL) {
                fputs("else\n", out);
                el_hir_dump_stmt(node->as.if_.else_, indent + 1, out);
            }
            break;

        case EL_HIR_STMT_BLOCK:
            el_hir_dump_print_indent(indent, out);
            fputs("{\n", out);
            el_hir_dump_stmt(curr->as.block.stmts, indent + 1, out);
            el_hir_dump_print_indent(indent, out);
            fputs("}\n", out);
            continue;
        }
        EL_UNREACHABLE_ENUM_VAL(ElHirStmtKind, curr->kind);
    }
}
