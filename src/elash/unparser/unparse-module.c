#include <elash/unparser/unparser.h>

void el_unparser_unparse_module(ElUnparser* unpar, ElAstModule* module) {
    for (ElAstDecl* decl = module->head; decl != NULL; decl = decl->next) {
        el_unparser_unparse_decl(unpar, decl);
    }
    el_unparser_push_punct(unpar, EL_TT_EOF);
}
