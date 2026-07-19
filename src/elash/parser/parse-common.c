#include <elash/parser/parser.h>

#include <elash/ast/tree/common/ident.h>

ElAstIdent* _el_parser_parse_ident(ElParser* parser) {
    if (!el_parser_check(parser, EL_TT_IDENT)) {
        el_parser_expect(parser, EL_TT_IDENT);
        return NULL;
    }

    ElToken tok = el_parser_advance(parser);
    return el_ast_new_ident_raw(parser->arena, tok.span, tok.lexeme);
}

bool _el_parser_lookahead_skip_type(ElParser* parser, usize* idx) {
    if (el_parser_peek_at(parser, *idx).type != EL_TT_IDENT) return false;
    (*idx)++;

    while (true) {
        ElToken tok = el_parser_peek_at(parser, *idx);
        if (tok.type == EL_TT_BITWISE_AND) {
            (*idx)++;
        } else if (tok.type == EL_TT_LBRACKET) {
            (*idx)++;
            int depth = 1;
            while (depth > 0) {
                tok = el_parser_peek_at(parser, (*idx)++);
                if (tok.type == EL_TT_EOF) return false;
                if (tok.type == EL_TT_LBRACKET) depth++;
                if (tok.type == EL_TT_RBRACKET) depth--;
            }
        } else {
            break;
        }
    }
    return true;
}
