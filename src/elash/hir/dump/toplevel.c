#include <elash/hir/dump/toplevel.h>
#include <elash/hir/dump/stmt.h>
#include <elash/hir/dump/indent.h>
#include <elash/sema/symbol/dump.h>

#include <elash/hir/tree/toplevel.h>
#include <elash/hir/tree/stmt.h>

#include <elash/sema/type.h>
#include <elash/defs/sv.h>

static void _el_hir_dump_func_sig(ElSymbol* symbol, usize indent, FILE* out, const char* prefix) {
    ElFuncSymbol* sym = &symbol->as.func;

    el_hir_dump_print_indent(indent, out);
    fprintf(out, "%s ", prefix);
    el_sema_dump_type(sym->ret_type, out);
    fputc(' ', out);
    el_sema_dump_symbol(symbol, out);
    fputc('(', out);

    for (usize i = 0; i < sym->param_count; ++i) {
        ElSymbol* param = sym->params[i];
        if (i > 0) fputs(", ", out);

        el_sema_dump_type(param->as.var.type, out);
        fputc(' ', out);
        el_sema_dump_symbol(param, out);
    }
    fputs(")", out);
}

void el_hir_dump_toplevel(ElHirTopLevel* node, usize indent, FILE* out) {
   switch (node->kind) {
   case EL_HIR_TOPLVL_FUNC_DEF: {
       ElHirFuncDef* func = &node->as.func_def;
       _el_hir_dump_func_sig(func->symbol, indent, out, "define");

       fputs(" {\n", out);
       for (ElHirStmt* stmt = func->block.stmts; stmt; stmt = stmt->next) {
           el_hir_dump_stmt(stmt, indent + 1, out);
       }
       el_hir_dump_print_indent(indent, out);
       fputs("}\n", out);
       break;
   }
   case EL_HIR_TOPLVL_FUNC_DECL: {
       _el_hir_dump_func_sig(node->as.func_decl.symbol, indent, out, "declare");
       fputs(";\n", out);
       break;
   }
   }
}
