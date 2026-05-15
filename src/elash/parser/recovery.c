#include <elash/parser/parser.h>

#include <elash/lexer/token.h>

static bool _el_parser_at_sync_point(ElTokenType tok, ElParserSyncKind kind, int brace_depth, int paren_depth) {
    if (tok == EL_TT_EOF) {
        return true;
    }

    switch (kind) {
    case EL_PARSER_SYNC_STMT:
        return tok == EL_TT_SEMICOLON || (brace_depth == 0 && tok == EL_TT_RBRACE);
    case EL_PARSER_SYNC_BLOCK:
        return brace_depth == 0 && tok == EL_TT_RBRACE;
    case EL_PARSER_SYNC_DECL:
        return brace_depth == 0 && tok == EL_TT_SEMICOLON;
    case EL_PARSER_SYNC_EXPR:
        return paren_depth == 0 && brace_depth == 0 && (
            tok == EL_TT_COMMA || tok == EL_TT_SEMICOLON || tok == EL_TT_RPAREN || tok == EL_TT_RBRACE
        );
    default:
        return false;
    }
}

static void _el_parser_skip_balanced(ElParser* parser, ElTokenType open, ElTokenType close) {
    if (!el_parser_check(parser, open)) {
        return;
    }

    int depth = 0;
    do {
        if (el_parser_check(parser, open)) {
            depth++;
        } else if (el_parser_check(parser, close)) {
            depth--;
        }
        el_parser_advance(parser);
    } while (depth > 0 && parser->current.type != EL_TT_EOF);
}

void* el_parser_sync(ElParser* parser, ElParserSyncKind kind) {
    if (parser->current.type == EL_TT_EOF) {
        return NULL;
    }

    int brace_depth = 0;
    int paren_depth = 0;

    while (true) {
        ElTokenType tok = parser->current.type;

        if (_el_parser_at_sync_point(tok, kind, brace_depth, paren_depth)) {
            if ((kind == EL_PARSER_SYNC_STMT || kind == EL_PARSER_SYNC_DECL) && tok == EL_TT_SEMICOLON) {
                el_parser_advance(parser);
            }
            return NULL;
        }

        if (tok == EL_TT_LBRACE) {
            brace_depth++;
        } else if (tok == EL_TT_RBRACE && brace_depth > 0) {
            brace_depth--;
        } else if (tok == EL_TT_LPAREN) {
            paren_depth++;
        } else if (tok == EL_TT_RPAREN && paren_depth > 0) {
            paren_depth--;
        }

        el_parser_advance(parser);

        if (parser->current.type == EL_TT_EOF) {
            return NULL;
        }
    }

    return NULL;
}

void el_parser_skip_to(ElParser* parser, ElTokenType type) {
    while (parser->current.type != type && parser->current.type != EL_TT_EOF) {
        el_parser_advance(parser);
    }
}

void el_parser_skip_balanced_parens(ElParser* parser) {
    _el_parser_skip_balanced(parser, EL_TT_LPAREN, EL_TT_RPAREN);
}

void el_parser_skip_balanced_braces(ElParser* parser) {
    _el_parser_skip_balanced(parser, EL_TT_LBRACE, EL_TT_RBRACE);
}
