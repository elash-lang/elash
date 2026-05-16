#include <elash/ast/dump/stmt.h>
#include <elash/ast/tree/stmt.h>

#include <elash/ast/dump/indent.h>
#include <elash/ast/dump/expr.h>
#include <elash/ast/dump/type.h>

void el_ast_dump_stmt(ElAstStmtNode* node, usize indent, FILE* out) {
    switch (node->type) {
    case EL_AST_STMT_EXPR:
        el_ast_dump_print_indent(indent, out);
        fprintf(out, "ExprStmt:\n");
        el_ast_dump_expr(node->as.expr, indent + 1, out);
        break;

    case EL_AST_STMT_RETURN:
        el_ast_dump_print_indent(indent, out);
        fprintf(out, "ReturnStmt:\n");
        if (node->as.return_.value) {
            el_ast_dump_expr(node->as.return_.value, indent + 1, out);
        } else {
            el_ast_dump_print_indent(indent + 1, out);
            fprintf(out, "void\n");
        }
        break;

    case EL_AST_STMT_IF:
        el_ast_dump_print_indent(indent, out);
        fprintf(out, "IfStmt:\n");
        el_ast_dump_print_indent(indent + 1, out);
        fprintf(out, "cond:\n");
        el_ast_dump_expr(node->as.if_.cond, indent + 2, out);
        el_ast_dump_print_indent(indent + 1, out);
        fprintf(out, "then:\n");
        el_ast_dump_stmt(node->as.if_.then, indent + 2, out);
        el_ast_dump_print_indent(indent + 1, out);
        if (node->as.if_.else_ != NULL) {
            fprintf(out, "else:\n");
            el_ast_dump_stmt(node->as.if_.else_, indent + 2, out);
        }
        break;

    case EL_AST_STMT_WHILE:
        el_ast_dump_print_indent(indent, out);
        fprintf(out, "WhileStmt:\n");
        el_ast_dump_print_indent(indent + 1, out);
        fprintf(out, "cond:\n");
        el_ast_dump_expr(node->as.while_.cond, indent + 2, out);
        el_ast_dump_print_indent(indent + 1, out);
        fprintf(out, "body:\n");
        el_ast_dump_stmt(node->as.while_.body, indent + 2, out);
        el_ast_dump_print_indent(indent + 1, out);
        break;

    case EL_AST_STMT_BREAK:
        el_ast_dump_print_indent(indent, out);
        fprintf(out, "BreakStmt\n");
        break;
    case EL_AST_STMT_CONTINUE:
        el_ast_dump_print_indent(indent, out);
        fprintf(out, "ContinueStmt\n");
        break;

    case EL_AST_STMT_BLOCK:
        el_ast_dump_print_indent(indent, out);
        fprintf(out, "BlockStmt:\n");
        for (ElAstStmtNode* stmt = node->as.block.stmts; stmt; stmt = stmt->next) {
            el_ast_dump_stmt(stmt, indent + 1, out);
        }
        break;

    case EL_AST_STMT_ASSIGN:
        el_ast_dump_print_indent(indent, out);
        fprintf(out, "AssignStmt:\n");
        el_ast_dump_print_indent(indent + 1, out);
        fputs("target:\n", out);
        el_ast_dump_expr(node->as.assign.target, indent + 2, out);
        el_ast_dump_print_indent(indent + 1, out);
        fputs("value:\n", out);
        el_ast_dump_expr(node->as.assign.value, indent + 2, out);
        break;
    case EL_AST_STMT_COMPOUND_ASSIGN:
        el_ast_dump_print_indent(indent, out);
        fprintf(out, "CompoundAssignStmt:\n");
        el_ast_dump_print_indent(indent + 1, out);
        fprintf(out, "op: " EL_SV_FMT, EL_SV_FARG(el_sema_bin_op_to_string(node->as.cassign.op)));
        el_ast_dump_print_indent(indent + 1, out);
        fputs("target:\n", out);
        el_ast_dump_expr(node->as.cassign.target, indent + 2, out);
        el_ast_dump_print_indent(indent + 1, out);
        fputs("value:\n", out);
        el_ast_dump_expr(node->as.cassign.value, indent + 2, out);
        break;

    case EL_AST_STMT_VAR_DEF:
        el_ast_dump_print_indent(indent, out);
        fprintf(out, "VarDefStmt:\n");
        el_ast_dump_print_indent(indent + 1, out);
        fprintf(out, "type:\n");
        el_ast_dump_type(node->as.var_def.type, indent + 2, out);
        el_ast_dump_print_indent(indent + 1, out);
        fprintf(out, "name: " EL_SV_FMT "\n", EL_SV_FARG(node->as.var_def.name->name));
        if (node->as.var_def.init != NULL) {
            el_ast_dump_print_indent(indent + 1, out);
            fprintf(out, "init:\n");
            el_ast_dump_expr(node->as.var_def.init, indent + 2, out);
        }
        break;
    }
}
