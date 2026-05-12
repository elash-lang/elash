#include <elash/ast/toplevel.h>
#include <elash/ast/expr.h>
#include <elash/ast/stmt.h>

#include <elash/util/assert.h>

#include <stdio.h>

void el_ast_dump_func_sig(const ElAstFuncSignature* sig, usize indent, FILE* out, const char* node_name) {
    el_ast_dump_print_indent(indent, out);
    fprintf(out, "%s(\""EL_SV_FMT"\", ret_type=\"", node_name, EL_SV_FARG(sig->name->name));
    el_ast_dump_type(sig->ret_type, out);
    fprintf(out, "\"):\n");

    el_ast_dump_print_indent(indent + 1, out);
    fprintf(out, "params:\n");
    for (ElAstFuncParam* param = sig->params.head; param; param = param->next) {
        el_ast_dump_print_indent(indent + 2, out);
        fprintf(out, "Param(\""EL_SV_FMT"\", type=\"", EL_SV_FARG(param->name->name));
        el_ast_dump_type(param->type, out);
        fprintf(out, "\")\n");
    }
}

void el_ast_dump_toplevel(ElAstTopLevelNode* node, usize indent, FILE* out) {
    switch (node->type) {
    case EL_AST_TOPLVL_FUNC_DEF:
        el_ast_dump_func_sig(&node->as.func_def.sig, indent, out, "FuncDef");
        el_ast_dump_print_indent(indent + 1, out);
        fprintf(out, "body:\n");
        if (node->as.func_def.block) {
            for (ElAstStmtNode* stmt = node->as.func_def.block->stmts; stmt; stmt = stmt->next) {
                el_ast_dump_stmt(stmt, indent + 2, out);
            }
        }
        return;
    case EL_AST_TOPLVL_FUNC_DECL:
        el_ast_dump_func_sig(&node->as.func_def.sig, indent, out, "FuncDecl");
        return;
    }
    EL_UNREACHABLE_ENUM_VAL(ElAstTopLevelType, node->type);
}

