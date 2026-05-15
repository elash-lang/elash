#include <elash/parser/parser.h>

#include <elash/ast/tree/common/type.h>
#include <elash/ast/tree/common/ident.h>

ElAstIdentNode* _el_parser_parse_ident(ElParser* parser) {
    ElToken tok = parser->current;
    el_parser_expect(parser, EL_TT_IDENT);
    if (el_parser_has_errs(parser)) return NULL;

    return el_ast_new_ident_node_raw(parser->arena, tok.span, tok.lexeme);
}

ElAstTypeNode* _el_parser_parse_type(ElParser* parser) {
    ElAstIdentNode* name = _el_parser_parse_ident(parser);
    if (el_parser_has_errs(parser)) return NULL;

    ElAstTypeNode* type = el_ast_new_type_name(parser->arena, name->span, name);

    while (el_parser_check(parser, EL_TT_STAR)) {
        ElToken star_tok = parser->current;
        el_parser_advance(parser);
        type = el_ast_new_type_ptr(parser->arena, el_source_span_merge(type->span, star_tok.span), type);
    }

    return type;
}
