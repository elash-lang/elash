#include <elash/parser/parser.h>
#include <elash/parser/utility.h>

#include <elash/ast/tree/common/type.h>
#include <elash/ast/tree/common/ident.h>

ElParserErrorCode _el_parser_parse_ident(ElParser* parser, ElAstIdentNode** out) {
    ElToken tok = parser->current;
    ElParserErrorCode result = el_parser_expect(parser, EL_TT_IDENT);
    if (result != EL_PARSER_ERR_OK) return result;

    if (out) {
        *out = el_ast_new_ident_node_raw(parser->arena, tok.span, tok.lexeme);
    }

    return _el_parser_ret_ok(parser);
}

ElParserErrorCode _el_parser_parse_type(ElParser* parser, ElAstTypeNode** out) {
    ElParserErrorCode result;

    ElAstIdentNode* name;
    result = _el_parser_parse_ident(parser, &name);
    if (result != EL_PARSER_ERR_OK) return result;

    ElAstTypeNode* type = el_ast_new_type_name(parser->arena, name->span, name);

    while (el_parser_check(parser, EL_TT_STAR)) {
        ElToken star_tok = parser->current;
        el_parser_advance(parser);
        type = el_ast_new_type_ptr(parser->arena, el_source_span_merge(type->span, star_tok.span), type);
    }

    if (out) {
        *out = type;
    }

    return _el_parser_ret_ok(parser);
}
