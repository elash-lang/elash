#include "parser-internals.h"

#include <elash/ast/tree/type.h>
#include <elash/diag/engine.h>

static ElMutabilitySpec parse_mut_spec(ElParser* parser, ElMutabilitySpec current, ElSourceSpan* span) {
    while (is_mut_spec_token(parser->current.type)) {
        ElToken tok = el_parser_advance(parser);
        if (current != EL_MUTSPEC_DEFAULT) {
            el_diag_report(
                parser->diag, EL_DIAG_ERROR, "syntax.duplicate-mutability",
                tok.span, "duplicate mutability specifier"
            );
        } else {
            current = mut_spec_from_token(tok.type);
            if (span != NULL) {
                *span = el_srcspan_is_valid(*span)
                    ? el_srcspan_merge(*span, tok.span)
                    : tok.span;
            }
        }
    }
    return current;
}

static ElAstType* parse_tuple_type(ElParser* parser, ElSourceSpan prefix_span, ElMutabilitySpec mut, ElToken struct_tok) {
    el_parser_advance(parser); // (

    ElAstType* head = NULL;
    ElAstType* tail = NULL;
    usize count = 0;

    if (!el_parser_check(parser, EL_TT_RPAREN)) {
        while (true) {
            ElAstType* elem = _el_parse_type(parser);
            if (elem == NULL) break;

            el_ast_type_list_append(&head, &tail, elem);
            count++;

            if (!el_parser_match(parser, EL_TT_COMMA)) break;
            if (el_parser_check(parser, EL_TT_RPAREN)) break;
        }
    }

    ElToken rparen_tok = el_parser_expect(parser, EL_TT_RPAREN);
    ElSourceSpan span = el_srcspan_is_valid(prefix_span)
        ? el_srcspan_merge(prefix_span, rparen_tok.span)
        : el_srcspan_merge(struct_tok.span, rparen_tok.span);

    return el_ast_new_type_tuple(parser->aarena, span, mut, head, count);
}

static ElAstType* parse_struct_type(ElParser* parser, ElSourceSpan prefix_span, ElMutabilitySpec mut, ElToken struct_tok) {
    el_parser_advance(parser); // {

    ElAstDecl* head = NULL;
    ElAstDecl* tail = NULL;
    usize count = 0;

    while (!el_parser_check(parser, EL_TT_RBRACE)) {
        ElAstDecl* elem = el_parse_decl(parser);
        if (elem == NULL) break;

        el_ast_append_decl(&head, &tail, elem);
        count++;
    }

    ElToken rbrace_tok = el_parser_expect(parser, EL_TT_RBRACE);
    ElSourceSpan span = el_srcspan_is_valid(prefix_span)
        ? el_srcspan_merge(prefix_span, rbrace_tok.span)
        : el_srcspan_merge(struct_tok.span, rbrace_tok.span);

    return el_ast_new_type_struct(parser->aarena, span, mut, head, count);
}

ElAstType* _el_parse_type_suffixes(ElParser* parser, ElAstType* type) {
    while (true) {
        if (el_parser_check(parser, EL_TT_BITWISE_AND)) {
            ElToken amp_tok = el_parser_advance(parser);
            ElSourceSpan span = el_srcspan_merge(type->span, amp_tok.span);
            ElMutabilitySpec mut = parse_mut_spec(parser, EL_MUTSPEC_DEFAULT, &span);
            type = el_ast_new_type_ref(parser->aarena, span, mut, type);
        } else if (el_parser_check(parser, EL_TT_OPT)) {
            ElToken opt_tok = el_parser_advance(parser);
            ElSourceSpan span = el_srcspan_merge(type->span, opt_tok.span);
            ElMutabilitySpec mut = parse_mut_spec(parser, EL_MUTSPEC_DEFAULT, &span);
            type = el_ast_new_type_opt(parser->aarena, span, mut, type);
        } else if (el_parser_check(parser, EL_TT_LBRACKET)) {
            el_parser_advance(parser); // '['

            // T[]
            if (el_parser_check(parser, EL_TT_RBRACKET)) {
                ElToken rbracket_tok = el_parser_advance(parser);
                ElSourceSpan span = el_srcspan_merge(type->span, rbracket_tok.span);
                ElMutabilitySpec mut = parse_mut_spec(parser, EL_MUTSPEC_DEFAULT, &span);
                type = el_ast_new_type_slice(parser->aarena, span, mut, type, false);
                continue;
            }

            // T[&]
            if (el_parser_check(parser, EL_TT_BITWISE_AND)
                && el_parser_peek_at(parser, 1).type == EL_TT_RBRACKET
            ) {
                el_parser_advance(parser); // &
                ElToken rbracket_tok = el_parser_expect(parser, EL_TT_RBRACKET);
                ElSourceSpan span = el_srcspan_merge(type->span, rbracket_tok.span);
                ElMutabilitySpec mut = parse_mut_spec(parser, EL_MUTSPEC_DEFAULT, &span);
                type = el_ast_new_type_slice(parser->aarena, span, mut, type, true);
                continue;
            }

            ElAstExpr* size = el_parse_expr(parser);
            ElToken rbracket = el_parser_expect(parser, EL_TT_RBRACKET);
            ElSourceSpan span = el_srcspan_merge(type->span, rbracket.span);
            ElMutabilitySpec mut = parse_mut_spec(parser, EL_MUTSPEC_DEFAULT, &span);
            type = el_ast_new_type_array(parser->aarena, span, mut, type, size);
        } else {
            break;
        }
    }

    return type;
}

ElAstType* _el_parse_type(ElParser* parser) {
    el_prof_begin_sub(parser->prof, parser->pss_type);

    ElSourceSpan prefix_mut_span = EL_SRCSPAN_NULL;
    ElMutabilitySpec prefix_mut = parse_mut_spec(parser, EL_MUTSPEC_DEFAULT, &prefix_mut_span);

    ElAstType* type;
    if (el_parser_check(parser, EL_TT_KW_STRUCT)) {
        ElToken struct_tok = el_parser_advance(parser);
        if (el_parser_check(parser, EL_TT_LPAREN)) {
            type = parse_tuple_type(parser, prefix_mut_span, prefix_mut, struct_tok);
        } else if (el_parser_check(parser, EL_TT_LBRACE)) {
            type = parse_struct_type(parser, prefix_mut_span, prefix_mut, struct_tok);
        } else {
            _el_parser_report_unexpected(parser, parser->current);
            el_prof_finish_sub(parser->prof, parser->pss_type);
            return NULL;
        }
    } else {
        ElAstIdent* name = _el_parse_ident(parser);
        if (name == NULL) {
            el_prof_finish_sub(parser->prof, parser->pss_type);
            return NULL;
        }

        ElSourceSpan span = el_srcspan_is_valid(prefix_mut_span)
            ? el_srcspan_merge(prefix_mut_span, name->span)
            : name->span;

        type = el_ast_new_type_name(parser->aarena, span, prefix_mut, name);
    }

    ElAstType* result = _el_parse_type_mut_and_suffixes(parser, type);
    el_prof_finish_sub(parser->prof, parser->pss_type);
    return result;
}

ElAstType* _el_parse_type_mut_and_suffixes(ElParser* parser, ElAstType* type) {
    type->mut = parse_mut_spec(parser, type->mut, &type->span);
    return _el_parse_type_suffixes(parser, type);
}
