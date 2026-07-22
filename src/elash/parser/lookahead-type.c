#include <elash/parser/parser.h>
#include <elash/ast/tree/type.h>

static bool lookahead_skip_balanced(ElParser* parser, usize* idx, ElTokenType open, ElTokenType close) {
    uint depth = 0;
    do {
        ElToken tok = el_parser_peek_at(parser, (*idx)++);

        if (tok.type == EL_TT_EOF) return false;

        if (tok.type == open)  depth++;
        if (tok.type == close) depth--;
    } while (depth > 0);

    return true;
}

static bool lookahead_skip_type_base(ElParser* parser, usize* idx) {
    ElToken tok = el_parser_peek_at(parser, *idx);

    if (tok.type == EL_TT_IDENT) {
        (*idx)++;
        return true;
    }

    if (tok.type != EL_TT_KW_STRUCT)
        return false;

    (*idx)++;

    tok = el_parser_peek_at(parser, *idx);
    if (tok.type == EL_TT_LPAREN)
        return lookahead_skip_balanced(parser, idx, EL_TT_LPAREN, EL_TT_RPAREN);
    if (tok.type == EL_TT_LBRACE)
        return lookahead_skip_balanced(parser, idx, EL_TT_LBRACE, EL_TT_RBRACE);

    return false;
}

static bool lookahead_skip_type_suffixes(ElParser* parser, usize* idx) {
    while (true) {
        ElToken tok = el_parser_peek_at(parser, *idx);

        if (tok.type == EL_TT_BITWISE_AND) {
            (*idx)++;
        } else if (tok.type == EL_TT_LBRACKET) {
            if (!lookahead_skip_balanced(parser, idx, EL_TT_LBRACKET, EL_TT_RBRACKET))
                return false;
        } else {
            break;
        }
    }

    return true;
}

bool _el_parser_lookahead_skip_type(ElParser* parser, usize* idx) {
    return lookahead_skip_type_base(parser, idx)
        && lookahead_skip_type_suffixes(parser, idx);
}
