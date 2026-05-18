#include <elash/ast/dump/module.h>
#include <elash/ast/tree/module.h>

#include <elash/ast/dump/toplevel.h>
#include <elash/ast/dump/indent.h>

void el_ast_dump_module(ElAstModule* mod, usize indent, FILE* out) {
    el_ast_dump_print_indent(indent, out);
    fprintf(out, "Module(count=%zu):\n", mod->count);
    for (ElAstTopLevel* node = mod->head; node != NULL; node = node->next) {
        el_ast_dump_toplevel(node, indent + 1, out);
    }
}
