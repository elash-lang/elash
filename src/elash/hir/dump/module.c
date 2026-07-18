#include <elash/hir/dump/module.h>
#include <elash/hir/dump/decl.h>
#include <elash/hir/dump/indent.h>
#include <elash/hir/tree/module.h>

void el_hir_dump_module(ElHirModule* module, usize indent, FILE* out) {
    for (ElHirDecl* node = module->head; node; node = node->next) {
        el_hir_dump_decl(node, indent, out);
        fputs("\n", out);
    }
}
