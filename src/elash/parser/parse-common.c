#include <elash/parser/parser.h>

#include <elash/ast/tree/common/type.h>
#include <elash/ast/tree/common/ident.h>

ElAstIdentNode* _el_parser_parse_ident(ElParser* parser) {
    if (!el_parser_check(parser, EL_TT_IDENT)) {
        el_parser_expect(parser, EL_TT_IDENT);
        return NULL;
    }

    ElToken tok = parser->current;
    el_parser_advance(parser);

    return el_ast_new_ident_node_raw(parser->arena, tok.span, tok.lexeme);
}

ElAstTypeNode* _el_parser_parse_type(ElParser* parser) {
    ElAstIdentNode* name = _el_parser_parse_ident(parser);
    if (name == NULL) return NULL;

    ElAstTypeNode* type = el_ast_new_type_name(parser->arena, name->span, name);

    while (true) {
        if (el_parser_check(parser, EL_TT_STAR)) {
            ElToken star_tok = parser->current;
            el_parser_advance(parser);
            type = el_ast_new_type_ptr(parser->arena, el_source_span_merge(type->span, star_tok.span), type);
        } else if (el_parser_check(parser, EL_TT_LBRACKET)) {
            el_parser_advance(parser); // '['

            // T[]
            if (el_parser_check(parser, EL_TT_RBRACKET)) {
                ElToken rbracket_tok = parser->current;
                el_parser_advance(parser);
                type = el_ast_new_type_slice(parser->arena, el_source_span_merge(type->span, rbracket_tok.span), type);
                continue;
            }

            // T[*]
            if (el_parser_check(parser, EL_TT_STAR)) {
                ElToken star_tok = parser->current;
                el_parser_advance(parser);
                type = el_ast_new_type_raw_slice(parser->arena, el_source_span_merge(type->span, star_tok.span), type);
                el_parser_expect(parser, EL_TT_RBRACKET);
                continue;
            }

            ElAstExprNode* size = el_parser_parse_expr(parser);
            ElToken rbracket = parser->current;
            el_parser_expect(parser, EL_TT_RBRACKET);
            type = el_ast_new_type_array(parser->arena, el_source_span_merge(type->span, rbracket.span), type, size);
        } else {
            break;
        }
    }

    return type;
}
