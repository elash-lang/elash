#include <elash/hir/dump/toplevel.h>
#include <elash/hir/dump/decl.h>

#include <elash/hir/tree/toplevel.h>
#include <elash/util/assert.h>

void el_hir_dump_toplevel(ElHirTopLevel* node, usize indent, FILE* out) {
   switch (node->kind) {
   case EL_HIR_TOPLVL_DECL:
       el_hir_dump_decl(node->as.decl, indent, out);
       break;
   }
}
