#include <elash/hir/dump/toplevel.h>
#include <elash/hir/dump/stmt.h>
#include <elash/hir/dump/indent.h>
#include <elash/sema/symbol/dump.h>

#include <elash/hir/tree/toplevel.h>

#include <elash/sema/type.h>
#include <elash/defs/sv.h>

void el_hir_dump_toplevel(ElHirTopLevelNode* node, usize indent, FILE* out) {
   switch (node->kind) {
   case EL_HIR_TOPLVL_FUNC_DEF: {
       ElHirFuncDef* func = &node->as.func_def; 
       ElFuncSymbol* sym = &func->symbol->as.func;

       el_hir_dump_print_indent(indent, out);
       fputs("define ", out);
       el_sema_dump_type(sym->ret_type, out);
       fputc(' ', out);
       el_sema_dump_symbol(func->symbol, out);
       fputc('(', out);
       
       for (usize i = 0; i < sym->param_count; ++i) {
           ElSymbol* param = sym->params[i];
           if (i > 0) fputs(", ", out);

           el_sema_dump_type(param->as.var.type, out);
           fputc(' ', out);
           el_sema_dump_symbol(param, out);
       }
       fputs(")", out);
       
       fputs(" {\n", out);
       el_hir_dump_stmt(func->block.stmts, indent + 1, out);
       el_hir_dump_print_indent(indent, out);
       fputs("}\n", out);
       break;
   }
   }
}
