#include <elash/parser/parser.h>

#include <elash/diag/meta.h>
#include <elash/lexer/token.h>

void _el_parser_report_expected(ElParser* parser, ElTokenType expected) {
    el_diag_report(
        parser->diag, EL_DIAG_ERROR, "syntax.expected-token",
        parser->current.span,
        "expected ${expected}, found ${found}",
        EL_DIAG_STRING("expected", el_token_type_format(expected)),
        EL_DIAG_STRING("found", el_token_type_format(parser->current.type))
    );
}

void _el_parser_report_unexpected(ElParser* parser, ElToken tok) {
    el_diag_report(
        parser->diag, EL_DIAG_ERROR, "syntax.unexpected-token",
        tok.span,
        "unexpected token: ${token}",
        EL_DIAG_STRING("token", el_token_type_format(tok.type))
    );
}

bool el_parser_has_errs(const ElParser* parser) {
    return el_diag_engine_has_errors(parser->diag);
}

uint el_parser_error_count(const ElParser* parser) {
    return parser->diag->summary.total_errors;
}

bool el_parser_had_new_errors(const ElParser* parser, uint error_count_before) {
    return parser->diag->summary.total_errors > error_count_before;
}

void el_parser_advance(ElParser* parser) {
    if (parser->has_lookahead) {
        parser->current = parser->lookahead;
        parser->has_lookahead = false;
    } else {
        parser->current = parser->tokens.next(&parser->tokens, parser->diag);
    }

    if (parser->current.type == EL_TT_UNKNOWN) {
        _el_parser_report_unexpected(parser, parser->current);
    }
}

ElToken el_parser_peek(ElParser* parser) {
    if (!parser->has_lookahead) {
        parser->lookahead = parser->tokens.next(&parser->tokens, parser->diag);
        parser->has_lookahead = true;
    }
    return parser->lookahead;
}

bool el_parser_match(ElParser* parser, ElTokenType type) {
    if (el_parser_check(parser, type)) {
        el_parser_advance(parser);
        return true;
    }
    return false;
}

bool el_parser_check(ElParser* parser, ElTokenType type) {
    return parser->current.type == type;
}

void el_parser_expect(ElParser* parser, ElTokenType type) {
    if (el_parser_check(parser, type)) {
        el_parser_advance(parser);
        return;
    }
    _el_parser_report_expected(parser, type);
}

void el_parser_init(ElParser* parser, ElTokenStream tokens, ElDiagEngine* engine, ElDynArena* arena) {
    parser->tokens = tokens;
    parser->diag = engine;
    parser->arena = arena;
    parser->current.type = EL_TT_UNKNOWN;
    parser->has_lookahead = false;

    el_parser_advance(parser);
}
