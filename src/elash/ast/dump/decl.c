#include <elash/ast/dump/decl.h>
#include <elash/ast/dump/stmt.h>
#include <elash/ast/tree/stmt.h>
#include <elash/ast/dump/indent.h>
#include <elash/ast/dump/type.h>
#include <elash/ast/dump/init.h>
#include <elash/util/assert.h>

static void el_ast_dump_func_sig(const ElAstFuncSignature* sig, usize indent, FILE* out, const char* node_name) {
    el_ast_dump_print_indent(indent, out);
    fprintf(out, "%s(\""EL_SV_FMT"\"):\n", node_name, EL_SV_FARG(sig->name->name));

    el_ast_dump_print_indent(indent + 1, out);
    fprintf(out, "ret_type:\n");
    el_ast_dump_type(sig->ret_type, indent + 2, out);

    el_ast_dump_print_indent(indent + 1, out);
    fprintf(out, "params:\n");
    for (ElAstFuncParam* param = sig->params.head; param; param = param->next) {
        el_ast_dump_print_indent(indent + 2, out);
        fprintf(out, "Param(\""EL_SV_FMT"\"):\n", EL_SV_FARG(param->name->name));
        el_ast_dump_print_indent(indent + 3, out);
        fprintf(out, "type:\n");
        el_ast_dump_type(param->type, indent + 4, out);
    }
}

void el_ast_dump_decl(ElAstDecl* node, usize indent, FILE* out) {
    switch (node->type) {
    case EL_AST_DECL_VAR_DEF:
        el_ast_dump_print_indent(indent, out);
        fprintf(out, "VarDef:\n");
        el_ast_dump_print_indent(indent + 1, out);
        fprintf(out, "type:\n");
        el_ast_dump_type(node->as.var_def.type, indent + 2, out);
        el_ast_dump_print_indent(indent + 1, out);
        fprintf(out, "name: " EL_SV_FMT "\n", EL_SV_FARG(node->as.var_def.name->name));
        if (node->as.var_def.init != NULL) {
            el_ast_dump_print_indent(indent + 1, out);
            fprintf(out, "init:\n");
            el_ast_dump_init(node->as.var_def.init, indent + 2, out);
        }
        break;

    case EL_AST_DECL_VAR_DECL:
        el_ast_dump_print_indent(indent, out);
        fprintf(out, "VarDecl:\n");
        el_ast_dump_print_indent(indent + 1, out);
        fprintf(out, "type:\n");
        el_ast_dump_type(node->as.var_decl.type, indent + 2, out);
        el_ast_dump_print_indent(indent + 1, out);
        fprintf(out, "name: " EL_SV_FMT "\n", EL_SV_FARG(node->as.var_decl.name->name));
        break;

    case EL_AST_DECL_FUNC_DEF:
        el_ast_dump_func_sig(&node->as.func_def.sig, indent, out, "FuncDef");
        el_ast_dump_print_indent(indent + 1, out);
        fprintf(out, "body:\n");
        if (node->as.func_def.block) {
            for (ElAstStmt* stmt = node->as.func_def.block->stmts; stmt; stmt = stmt->next) {
                el_ast_dump_stmt(stmt, indent + 2, out);
            }
        }
        break;

    case EL_AST_DECL_FUNC_DECL:
        el_ast_dump_func_sig(&node->as.func_decl.sig, indent, out, "FuncDecl");
        break;
    }
}
