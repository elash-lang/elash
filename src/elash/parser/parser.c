#include <elash/parser/parser.h>
#include <elash/parser/utility.h>

#include <elash/util/dynarena.h>
#include <elash/util/strconv.h>

#include <elash/ast/expr.h>
#include <elash/ast/expr/bin.h>
#include <elash/ast/expr/unary.h>
#include <elash/ast/expr/literal.h>

ElParserErrorCode el_parser_advance(ElParser* parser) {
    if (parser->has_lookahead) {
        parser->current = parser->lookahead;
        parser->has_lookahead = false;
    } else {
        parser->current = parser->tokens.next(&parser->tokens, parser->engine);
    }
    
    if (parser->current.type == EL_TT_UNKNOWN) {
        return _el_parser_ret_err(parser, .code = EL_PARSER_ERR_UNEXPECTED_TOKEN, .token = parser->current);
    }

    return _el_parser_ret_ok(parser);
}

ElToken el_parser_peek(ElParser* parser) {
    if (!parser->has_lookahead) {
        parser->lookahead = parser->tokens.next(&parser->tokens, parser->engine);
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

ElParserErrorCode el_parser_expect(ElParser* parser, ElTokenType type) {
    if (el_parser_check(parser, type)) {
        return el_parser_advance(parser);
    }
    return _el_parser_ret_err(parser,
        .code = EL_PARSER_ERR_EXPECTED_TOKEN,
        .expected = type
    );
}

void el_parser_init(ElParser* parser, ElTokenStream tokens, ElDiagEngine* engine, ElDynArena* arena) {
    parser->tokens = tokens;
    parser->engine = engine;
    parser->arena = arena;
    parser->current.type = EL_TT_UNKNOWN;
    parser->has_lookahead = false;
    memset(&parser->last_err_details, 0, sizeof(parser->last_err_details));
}

ElParserErrorCode el_parser_parse(ElParser* parser, ElAstModuleNode** out) {
    if (parser->current.type == EL_TT_UNKNOWN) {
        ElParserErrorCode err = el_parser_advance(parser);
        if (err != EL_PARSER_ERR_OK) return err;
    }
    
    return _el_parser_parse_module(parser, out);
}
