#include <elash/parser/parser.h>
#include <elash/ast/tree/common/init.h>

ElAstInitializer* el_parser_parse_initializer(ElParser* parser) {
    if (el_parser_check(parser, EL_TT_LBRACE)) {
        ElToken lbrace = parser->current;
        el_parser_advance(parser);

        ElAstInitializer* head = NULL;
        ElAstInitializer* tail = NULL;
        usize count = 0;

        if (!el_parser_check(parser, EL_TT_RBRACE)) {
            while (true) {
                ElAstInitializer* init = el_parser_parse_initializer(parser);
                if (!init) break;

                el_ast_init_list_append(&head, &tail, init);
                count++;

                if (!el_parser_match(parser, EL_TT_COMMA)) break;
            }
        }

        ElToken rbrace = parser->current;
        el_parser_expect(parser, EL_TT_RBRACE);

        return el_ast_new_init_list(parser->arena, el_source_span_merge(lbrace.span, rbrace.span), head, count);
    }

    ElAstExprNode* expr = el_parser_parse_expr(parser);
    if (!expr) return NULL;
    return el_ast_new_init_expr(parser->arena, expr);
}

