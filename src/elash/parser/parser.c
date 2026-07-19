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

ElToken el_parser_advance(ElParser* parser) {
    ElToken prev = parser->current;

    if (parser->lookahead.len > 0) {
        el_tkque_pop(&parser->lookahead, &parser->current);
    } else {
        parser->current = parser->tokens.next(&parser->tokens, parser->diag);
    }

    if (parser->current.type == EL_TT_UNKNOWN) {
        _el_parser_report_unexpected(parser, parser->current);
    }

    return prev;
}

ElToken el_parser_peek(ElParser* parser) {
    return el_parser_peek_at(parser, 1);
}

ElToken el_parser_peek_at(ElParser* parser, usize n) {
    if (n == 0) return parser->current;

    while (parser->lookahead.len < n) {
        ElToken tok = parser->tokens.next(&parser->tokens, parser->diag);
        el_tkque_push(&parser->lookahead, tok);
    }

    ElToken out;
    el_tkque_at(&parser->lookahead, n - 1, &out);
    return out;
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

ElToken el_parser_expect(ElParser* parser, ElTokenType type) {
    if (el_parser_check(parser, type)) {
        return el_parser_advance(parser);
    }

    _el_parser_report_expected(parser, type);
    return parser->current;
}

void el_parser_init(ElParser* parser, ElTokenStream tokens, ElDiagEngine* engine, ElDynArena* arena) {
    parser->tokens = tokens;
    parser->diag = engine;
    parser->arena = arena;
    parser->current.type = EL_TT_UNKNOWN;
    el_tkque_init(&parser->lookahead);

    el_parser_advance(parser);
}

void el_parser_destroy(ElParser* parser) {
    el_tkque_destroy(&parser->lookahead);
}
