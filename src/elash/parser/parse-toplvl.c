#include <elash/parser/parser.h>
#include <elash/ast/tree/toplevel.h>

ElAstTopLevel* el_parser_parse_toplevel(ElParser* parser) {
    ElAstDecl* decl = el_parser_parse_decl(parser);
    if (decl == NULL) return NULL;
    return el_ast_new_toplevel_decl(parser->arena, decl);
}
