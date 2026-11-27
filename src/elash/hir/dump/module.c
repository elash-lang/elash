#include <elash/hir/dump/module.h>
#include <elash/hir/dump/decl.h>
#include <elash/hir/dump/indent.h>
#include <elash/hir/tree/module.h>

void el_hir_dump_module(ElHirModule* module, usize indent, ElWriter out) {
    for (ElHirDecl* node = module->head; node; node = node->next) {
        el_hir_dump_decl(node, indent, out);
        el_write_char(out, '\n');
    }
}
