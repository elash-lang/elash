#include "parser-internals.h"

#include <elash/ast/tree/init.h>
#include <elash/sema/intparse.h>

static bool is_designator_start(ElParser* parser) {
    return el_parser_check(parser, EL_TT_DOT) || el_parser_check(parser, EL_TT_LBRACKET);
}

static ElAstInit* parse_init_list(ElParser* parser, ElToken lbrace_tok) {
    ElAstInit* head = NULL;
    ElAstInit* tail = NULL;
    usize count = 0;

    while (true) {
        ElAstInit* init = el_parser_parse_init(parser);
        if (init == NULL) break;

        el_ast_init_list_append(&head, &tail, init);
        count++;

        if (!el_parser_match(parser, EL_TT_COMMA)) break;
        if (el_parser_check(parser, EL_TT_RBRACE)) break;
    }

    ElToken rbrace_tok = el_parser_expect(parser, EL_TT_RBRACE);
    return el_ast_new_init_list(parser->aarena, el_srcspan_merge(lbrace_tok.span, rbrace_tok.span), head, count);
}

static ElAstDesignator* parse_designator(ElParser* parser) {
    if (el_parser_check(parser, EL_TT_DOT)) {
        ElToken dot_tok = el_parser_advance(parser);
        ElToken tok = parser->current;
        if (tok.type == EL_TT_IDENT) {
            el_parser_advance(parser);
            ElSourceSpan span = el_srcspan_merge(dot_tok.span, tok.span);
            return el_ast_new_desig_member(parser->aarena, span, tok.lexeme);
        }

        if (tok.type == EL_TT_INT_LITERAL) {
            el_parser_advance(parser);

            usize index;
            if (!_el_parser_parse_const_idx(parser, tok, &index)) {
                return NULL;
            }
            ElSourceSpan span = el_srcspan_merge(dot_tok.span, tok.span);
            return el_ast_new_desig_tmember(parser->aarena, span, index);
        }

        _el_parser_report_unexpected(parser, tok);
        el_parser_advance(parser); // very advanced error recovery mechanism frfr
        return NULL;
    }

    // it's already guaranteed by is_designator_start
    // but maybe expect here would be better anyway
    // who cares
    ElToken lbracket_tok = el_parser_advance(parser); // '['
    ElAstExpr* index = el_parser_parse_expr(parser);
    ElToken rbracket_tok = el_parser_expect(parser, EL_TT_RBRACKET); // ']'

    ElSourceSpan span = el_srcspan_merge(lbracket_tok.span, rbracket_tok.span);
    return el_ast_new_desig_index(parser->aarena, span, index);
}

static ElAstDesigInitElem* parse_desig_init_elem(ElParser* parser) {
    ElAstDesignator* head = NULL;
    ElAstDesignator* tail = NULL;
    usize desig_count = 0;

    while (is_designator_start(parser)) {
        ElAstDesignator* desig = parse_designator(parser);
        if (desig != NULL) {
            el_ast_desig_list_append(&head, &tail, desig);
            desig_count++;
        }
    }

    el_parser_expect(parser, EL_TT_ASSIGN);
    ElAstInit* init = el_parser_parse_init(parser);

    ElSourceSpan start_span = head != NULL ? head->span : (init ? init->span : EL_SRCSPAN_NULL);
    ElSourceSpan end_span   = init != NULL ? init->span : (head ? head->span : EL_SRCSPAN_NULL);
    ElSourceSpan elem_span  = el_srcspan_merge(start_span, end_span);

    return el_ast_new_desig_init_elem(parser->aarena, elem_span, head, desig_count, init);
}

static ElAstInit* parse_designated_init(ElParser* parser, ElToken lbrace_tok) {
    ElAstDesigInitElem* head = NULL;
    ElAstDesigInitElem* tail = NULL;
    usize count = 0;

    while (true) {
        ElAstDesigInitElem* elem = parse_desig_init_elem(parser);
        if (elem == NULL) break;

        el_ast_desig_init_append(&head, &tail, elem);
        count++;

        if (!el_parser_match(parser, EL_TT_COMMA)) break;
        if (el_parser_check(parser, EL_TT_RBRACE)) break;
    }

    ElToken rbrace_tok = el_parser_expect(parser, EL_TT_RBRACE);
    return el_ast_new_desig_init(parser->aarena, el_srcspan_merge(lbrace_tok.span, rbrace_tok.span), head, count);
}

static ElAstInit* _parse_init_internal(ElParser* parser) {
    if (el_parser_check(parser, EL_TT_LBRACE)) {
        ElToken lbrace_tok = el_parser_advance(parser);

        if (el_parser_check(parser, EL_TT_RBRACE)) {
            ElToken rbrace_tok = el_parser_advance(parser);
            return el_ast_new_init_empty(parser->aarena, el_srcspan_merge(lbrace_tok.span, rbrace_tok.span));
        }

        if (is_designator_start(parser)) {
            return parse_designated_init(parser, lbrace_tok);
        }

        return parse_init_list(parser, lbrace_tok);
    }

    ElAstExpr* expr = el_parser_parse_expr(parser);
    if (expr == NULL) return NULL;
    return el_ast_new_init_expr(parser->aarena, expr);
}

ElAstInit* el_parser_parse_init(ElParser* parser) {
    el_prof_begin_sub(parser->prof, parser->pss_init);
    ElAstInit* result = _parse_init_internal(parser);
    el_prof_finish_sub(parser->prof, parser->pss_init);
    return result;
}
