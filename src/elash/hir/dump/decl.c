#include <elash/hir/dump/decl.h>
#include <elash/hir/dump/stmt.h>
#include <elash/hir/dump/expr.h>
#include <elash/hir/dump/indent.h>
#include <elash/hir/symbol/dump.h>

#include <elash/hir/tree/stmt.h>
#include <elash/hir/type.h>
#include <elash/util/assert.h>

static void el_hir_dump_func_sig(ElHirSymbol* symbol, usize indent, FILE* out, const char* prefix) {
    ElHirFuncSymbol* sym = &symbol->as.func;

    el_hir_dump_print_indent(indent, out);
    fprintf(out, "%s ", prefix);
    el_sema_dump_type(sym->type->as.func.ret_type, out);
    fputc(' ', out);
    el_sema_dump_symbol(symbol, out);
    fputc('(', out);

    for (usize i = 0; i < sym->param_count; ++i) {
        ElHirSymbol* param = sym->params[i];
        if (i > 0) fputs(", ", out);

        el_sema_dump_type(param->as.var.type, out);
        fputc(' ', out);
        el_sema_dump_symbol(param, out);
    }
    fputs(")", out);
}

void el_hir_dump_decl(ElHirDecl* node, usize indent, FILE* out) {
    switch (node->kind) {
    case EL_HIR_DECL_VAR_DEF: {
        ElHirVarDef* var_def = &node->as.var_def;
        el_hir_dump_print_indent(indent, out);
        fputs("define ", out);
        el_sema_dump_type(var_def->var->as.var.type, out);
        fputc(' ', out);
        el_sv_print(var_def->var->name, out);
        if (var_def->init != NULL) {
            fputs(" = ", out);
            el_hir_dump_expr(var_def->init, 0, out);
        }
        fputs(";\n", out);
        break;
    }
    case EL_HIR_DECL_VAR_DECL: {
        ElHirVarDecl* var_decl = &node->as.var_decl;
        el_hir_dump_print_indent(indent, out);
        fputs("declare ", out);
        el_sema_dump_type(var_decl->var->as.var.type, out);
        fputc(' ', out);
        el_sv_print(var_decl->var->name, out);
        fputs(";\n", out);
        break;
    }
    case EL_HIR_DECL_FUNC_DEF: {
        ElHirFuncDef* func = &node->as.func_def;
        el_hir_dump_func_sig(func->symbol, indent, out, "define");
        fputs(" {\n", out);
        for (ElHirStmt* stmt = func->block.stmts; stmt; stmt = stmt->next) {
            el_hir_dump_stmt(stmt, indent + 1, out);
        }
        el_hir_dump_print_indent(indent, out);
        fputs("}\n", out);
        break;
    }
    case EL_HIR_DECL_FUNC_DECL: {
        el_hir_dump_func_sig(node->as.func_decl.symbol, indent, out, "declare");
        fputs(";\n", out);
        break;
    }
    }
}
