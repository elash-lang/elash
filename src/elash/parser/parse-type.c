#include <elash/parser/parser.h>
#include <elash/ast/tree/type.h>

ElAstType* _el_parser_parse_type(ElParser* parser) {
    ElAstIdent* name = _el_parser_parse_ident(parser);
    if (name == NULL) return NULL;

    ElAstType* type = el_ast_new_type_name(parser->arena, name->span, name);

    while (true) {
        if (el_parser_check(parser, EL_TT_BITWISE_AND)) {
            ElToken amp_tok = parser->current;
            el_parser_advance(parser);
            type = el_ast_new_type_ref(parser->arena, el_source_span_merge(type->span, amp_tok.span), type);
        } else if (el_parser_check(parser, EL_TT_LBRACKET)) {
            el_parser_advance(parser); // '['

            // T[]
            if (el_parser_check(parser, EL_TT_RBRACKET)) {
                ElToken rbracket_tok = parser->current;
                el_parser_advance(parser);
                type = el_ast_new_type_slice(parser->arena, el_source_span_merge(type->span, rbracket_tok.span), type, false);
                continue;
            }

            // T[&]
            if (el_parser_check(parser, EL_TT_BITWISE_AND)) {
                ElToken amp_tok = parser->current;
                el_parser_advance(parser);
                type = el_ast_new_type_slice(parser->arena, el_source_span_merge(type->span, amp_tok.span), type, true);
                el_parser_expect(parser, EL_TT_RBRACKET);
                continue;
            }

            ElAstExpr* size = el_parser_parse_expr(parser);
            ElToken rbracket = parser->current;
            el_parser_expect(parser, EL_TT_RBRACKET);
            type = el_ast_new_type_array(parser->arena, el_source_span_merge(type->span, rbracket.span), type, size);
        } else {
            break;
        }
    }

    return type;
}
