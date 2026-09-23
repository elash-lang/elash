#include "parser-internals.h"

#include <elash/ast/tree/common/ident.h>

ElAstIdent* _el_parse_ident(ElParser* parser) {
    if (!el_parser_check(parser, EL_TT_IDENT)) {
        el_parser_expect(parser, EL_TT_IDENT);
        return NULL;
    }

    ElToken tok = el_parser_advance(parser);
    return el_ast_new_ident_raw(parser->aarena, tok.span, tok.lexeme);
}
