#include <elash/hir/dump/decl.h>
#include <elash/hir/dump/stmt.h>
#include <elash/hir/dump/expr.h>
#include <elash/hir/dump/indent.h>
#include <elash/hir/symbol/dump.h>

#include <elash/hir/tree/stmt.h>
#include <elash/hir/type.h>
#include <elash/util/assert.h>

static void el_hir_dump_func_sig(ElHirSymbol* symbol, usize indent, ElWriter out, const char* prefix) {
    ElHirFuncSymbol* sym = &symbol->as.func;

    el_hir_dump_print_indent(indent, out);
    el_writef(out, "%s ", prefix);
    el_hir_dump_type(sym->type->as.func.ret_type, out);
    el_write_char(out, ' ');
    el_dump_symbol(symbol, out);
    el_write_char(out, '(');

    for (usize i = 0; i < sym->param_count; ++i) {
        ElHirSymbol* param = sym->params[i];
        if (i > 0) el_swrite(out, ", ");

        el_hir_dump_type(param->as.var.type, out);
        el_write_char(out, ' ');
        el_dump_symbol(param, out);
    }
    el_write_char(out, ')');
}

void el_hir_dump_decl(ElHirDecl* node, usize indent, ElWriter out) {
    switch (node->kind) {
    case EL_HIR_DECL_NONE:
        break;

    case EL_HIR_DECL_VAR_DEF: {
        ElHirVarDef* var_def = &node->as.var_def;
        el_hir_dump_print_indent(indent, out);
        el_swrite(out, "define ");
        el_hir_dump_type(var_def->var->as.var.type, out);
        el_write_char(out, ' ');
        el_write(out, var_def->var->name);
        if (var_def->init != NULL) {
            el_swrite(out, " = ");
            el_hir_dump_expr(var_def->init, 0, out);
        }
        el_swrite(out, ";\n");
        break;
    }
    case EL_HIR_DECL_VAR_DECL: {
        ElHirVarDecl* var_decl = &node->as.var_decl;
        el_hir_dump_print_indent(indent, out);
        el_swrite(out, "declare ");
        el_hir_dump_type(var_decl->var->as.var.type, out);
        el_write_char(out, ' ');
        el_write(out, var_decl->var->name);
        el_swrite(out, ";\n");
        break;
    }
    case EL_HIR_DECL_FUNC_DEF: {
        ElHirFuncDef* func = &node->as.func_def;
        el_hir_dump_func_sig(func->symbol, indent, out, "define");
        el_swrite(out, " {\n");
        for (ElHirStmt* stmt = func->block.stmts; stmt; stmt = stmt->next) {
            el_hir_dump_stmt(stmt, indent + 1, out);
        }
        el_hir_dump_print_indent(indent, out);
        el_swrite(out, "}\n");
        break;
    }
    case EL_HIR_DECL_FUNC_DECL: {
        el_hir_dump_func_sig(node->as.func_decl.symbol, indent, out, "declare");
        el_swrite(out, ";\n");
        break;
    }
    }
}
