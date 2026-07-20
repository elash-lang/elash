#include <elash/parser/parser.h>
#include <elash/ast/tree/type.h>

static ElAstType* parse_tuple_type(ElParser* parser, ElToken struct_tok) {
    el_parser_advance(parser); // (

    ElAstType* head = NULL;
    ElAstType* tail = NULL;
    usize count = 0;

    if (!el_parser_check(parser, EL_TT_RPAREN)) {
        while (true) {
            ElAstType* elem = _el_parser_parse_type(parser);
            if (elem == NULL) break;

            el_ast_type_list_append(&head, &tail, elem);
            count++;

            if (!el_parser_match(parser, EL_TT_COMMA)) break;
            if (el_parser_check(parser, EL_TT_RPAREN)) break;
        }
    }

    ElToken rparen_tok = el_parser_expect(parser, EL_TT_RPAREN);
    return el_ast_new_type_tuple(parser->arena, el_source_span_merge(struct_tok.span, rparen_tok.span), head, count);
}

static ElAstType* parse_struct_type(ElParser* parser, ElToken struct_tok) {
    el_parser_advance(parser); // {

    ElAstDecl* head = NULL;
    ElAstDecl* tail = NULL;
    usize count = 0;

    while (!el_parser_check(parser, EL_TT_RBRACE)) {
        ElAstDecl* elem = el_parser_parse_decl(parser);
        if (elem == NULL) break;

        el_ast_append_decl(&head, &tail, elem);
        count++;
    }

    ElToken rbrace_tok = el_parser_expect(parser, EL_TT_RBRACE);
    return el_ast_new_type_struct(parser->arena, el_source_span_merge(struct_tok.span, rbrace_tok.span), head, count);
}

ElAstType* _el_parser_parse_type(ElParser* parser) {
    ElAstType* type;
    if (el_parser_check(parser, EL_TT_KW_STRUCT)) {
        ElToken struct_tok = el_parser_advance(parser);
        if (el_parser_check(parser, EL_TT_LPAREN)) {
            type = parse_tuple_type(parser, struct_tok);
        } else if (el_parser_check(parser, EL_TT_LBRACE)) {
            type= parse_struct_type(parser, struct_tok);
        }
    } else {
        ElAstIdent* name = _el_parser_parse_ident(parser);
        if (name == NULL) return NULL;
        type = el_ast_new_type_name(parser->arena, name->span, name);
    }

    while (true) {
        if (el_parser_check(parser, EL_TT_BITWISE_AND)) {
            ElToken amp_tok = el_parser_advance(parser);
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
                ElToken amp_tok = el_parser_advance(parser);
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
