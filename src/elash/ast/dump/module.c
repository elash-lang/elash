#include <elash/ast/dump/module.h>
#include <elash/ast/tree/module.h>

#include <elash/ast/dump/decl.h>
#include <elash/ast/dump/indent.h>

void el_ast_dump_module(ElAstModule* mod, usize indent, FILE* out) {
    el_ast_dump_print_indent(indent, out);
    fprintf(out, "Module(count=%zu):\n", mod->count);
    for (ElAstDecl* decl = mod->head; decl != NULL; decl = decl->next) {
        el_ast_dump_decl(decl, indent + 1, out);
    }
}
