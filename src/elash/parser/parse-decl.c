#include "parser-internals.h"

#include <elash/diag/engine.h>
#include <elash/diag/meta.h>
#include <elash/lexer/token.h>

static ElAstFuncParamList parse_func_params(ElParser* parser) {
    ElAstFuncParamList params = el_ast_make_func_param_list();

    while (parser->current.type != EL_TT_RPAREN && parser->current.type != EL_TT_EOF) {
        ElAstType* p_type = _el_parse_type(parser);
        if (p_type == NULL) {
            el_parser_skip_to(parser, EL_TT_RPAREN);
            return params;
        }

        ElAstIdent* p_name = _el_parse_ident(parser);
        if (p_name == NULL) {
            el_parser_skip_to(parser, EL_TT_RPAREN);
            return params;
        }

        el_ast_func_param_list_append(&params, el_ast_new_func_param(
            parser->aarena,
            el_srcspan_merge(p_type->span, p_name->span),
            p_type, p_name
        ));

        if (el_parser_match(parser, EL_TT_COMMA)) {
            continue;
        } else if (parser->current.type != EL_TT_RPAREN) {
            el_parser_expect(parser, EL_TT_RPAREN);
            return params;
        }
    }

    return params;
}

static ElAstFuncSignature parse_func_sig(ElParser* parser) {
    ElAstFuncSignature sig = {0};

    ElAstType* ret_type = _el_parse_type(parser);
    if (ret_type == NULL) return sig;

    ElAstIdent* name = _el_parse_ident(parser);
    if (name == NULL) return sig;

    el_parser_expect(parser, EL_TT_LPAREN);
    if (el_parser_has_errs(parser)) return sig;

    ElAstFuncParamList params = parse_func_params(parser);
    if (el_parser_has_errs(parser)) return sig;

    ElToken rparen_tok = parser->current;
    el_parser_expect(parser, EL_TT_RPAREN);
    if (el_parser_has_errs(parser)) return sig;

    ElSourceSpan span = el_srcspan_merge(ret_type->span, rparen_tok.span);
    return el_ast_func_signature(span, ret_type, name, params);
}

static ElAstDecl* parse_func_internal_decl(ElParser* parser, ElAstFuncSignature sig) {
    if (el_parser_check(parser, EL_TT_LBRACE)) {
        ElToken lbrace_tok = parser->current;
        el_parser_advance(parser);

        ElAstStmt* body_stmt = _el_parse_block(parser, lbrace_tok);
        ElSourceSpan span = el_srcspan_merge(sig.span, body_stmt->span);

        return el_ast_new_func_def(parser->aarena, span, sig, &body_stmt->as.block);
    }

    ElToken semi_tok = parser->current;
    el_parser_expect(parser, EL_TT_SEMICOLON);

    ElSourceSpan span = el_srcspan_merge(sig.span, semi_tok.span);
    return el_ast_new_func_decl(parser->aarena, span, sig);
}

static ElAstDeclarator* parse_declarator_list(ElParser* parser, bool allow_init) {
    ElAstDeclarator* head = NULL;
    ElAstDeclarator* tail = NULL;

    while (true) {
        ElAstIdent* name = _el_parse_ident(parser);
        if (name == NULL) return NULL;

        ElAstInit* init = NULL;
        if (el_parser_check(parser, EL_TT_ASSIGN)) {
            if (!allow_init) {
                el_diag_report(
                    parser->diag, EL_DIAG_ERROR, "syntax.extern-init",
                    parser->current.span,
                    "extern variables cannot have an initializer"
                );
            }

            // this is intentionally reachable, even tho initializers are not allowed here,
            // we still need to parse it to skip the tokens and report diagnostics
            el_parser_advance(parser);
            init = el_parse_init(parser);

            if (init == NULL) return NULL;
            if (!allow_init) init = NULL;
        }

        el_ast_append_declarator(&head, &tail,
            el_ast_new_declarator(parser->aarena, name, init));

        if (!el_parser_match(parser, EL_TT_COMMA)) break;
    }

    return head;
}

static ElAstDecl* parse_var_internal_decl(ElParser* parser) {
    bool is_static = el_parser_match(parser, EL_TT_KW_STATIC);

    ElAstType* type = _el_parse_type(parser);
    if (type == NULL) return el_parser_sync(parser, EL_PARSER_SYNC_DECL);

    ElAstDeclarator* declarators = parse_declarator_list(parser, /*allow_init=*/true);
    if (declarators == NULL) return el_parser_sync(parser, EL_PARSER_SYNC_DECL);

    ElToken semi_tok = parser->current;
    el_parser_expect(parser, EL_TT_SEMICOLON);

    ElSourceSpan span = el_srcspan_merge(type->span, semi_tok.span);
    return el_ast_new_var_def(parser->aarena, span, type, declarators, is_static);
}

static ElAstDecl* _el_parser_report_incomplete_decl(ElParser* parser, usize idx) {
    if (el_parser_peek_at(parser, idx).type != EL_TT_IDENT) {
        _el_parser_report_expected_at(parser, EL_TT_IDENT, idx);
        (void)_el_parse_type(parser);
    } else {
        el_parser_expect(parser, EL_TT_IDENT);
    }
    return el_parser_sync(parser, EL_PARSER_SYNC_DECL);
}

static ElAstDecl* parse_extern_decl(ElParser* parser, ElToken extern_tok) {
    usize idx = 0;
    if (!_el_parser_lookahead_skip_type(parser, &idx)) {
        return _el_parser_report_incomplete_decl(parser, idx);
    }
    if (el_parser_peek_at(parser, idx).type != EL_TT_IDENT) {
        return _el_parser_report_incomplete_decl(parser, idx);
    }

    if (el_parser_peek_at(parser, idx + 1).type == EL_TT_LPAREN) {
        ElAstFuncSignature sig = parse_func_sig(parser);
        if (sig.ret_type == NULL || sig.name == NULL) return el_parser_sync(parser, EL_PARSER_SYNC_DECL);

        if (el_parser_check(parser, EL_TT_LBRACE)) {
            el_diag_report(
                parser->diag, EL_DIAG_ERROR, "syntax.extern-body",
                parser->current.span,
                "extern functions cannot have a body"
            );
            el_parser_skip_balanced_braces(parser);
            return NULL;
        }

        ElToken semi_tok = parser->current;
        el_parser_expect(parser, EL_TT_SEMICOLON);

        ElSourceSpan span = el_srcspan_merge(extern_tok.span, semi_tok.span);
        return el_ast_new_func_decl(parser->aarena, span, sig);
    } else {
        ElAstType* type = _el_parse_type(parser);
        if (type == NULL) return el_parser_sync(parser, EL_PARSER_SYNC_DECL);

        ElAstDeclarator* declarators = parse_declarator_list(parser, /*allow_init=*/false);
        if (declarators == NULL) return el_parser_sync(parser, EL_PARSER_SYNC_DECL);

        ElToken semi_tok = parser->current;
        el_parser_expect(parser, EL_TT_SEMICOLON);

        ElSourceSpan span = el_srcspan_merge(extern_tok.span, semi_tok.span);
        return el_ast_new_var_decl(parser->aarena, span, type, declarators);
    }
}

// NOLINTBEGIN(bugprone-macro-parentheses): clueless
// maybe macros aren't the best way of deduplicating code but i really had
// no other ideas and... it's still way better than 35 lines duplicated x2!
#define PARSE_GENERIC_LIST(                                                                     \
    ElemType, TargetType, PARSE_TARGET, APPEND_FN,                                              \
    NEW_ELEM_FN, NEW_FN, TOKEN, IS_OPTIONAL                                                     \
) do {                                                                                          \
    ElemType* head = NULL;                                                                      \
    ElemType* tail = NULL;                                                                      \
                                                                                                \
    while (true) {                                                                              \
        ElToken name_tok = el_parser_expect(parser, EL_TT_IDENT);                               \
        if (el_parser_has_errs(parser)) return el_parser_sync(parser, EL_PARSER_SYNC_DECL);     \
                                                                                                \
        TargetType* target = NULL;                                                              \
        if (IS_OPTIONAL) {                                                                      \
            if (el_parser_match(parser, TOKEN)) {                                               \
                target = PARSE_TARGET(parser);                                                  \
                if (target == NULL) return el_parser_sync(parser, EL_PARSER_SYNC_DECL);         \
            }                                                                                   \
        } else {                                                                                \
            el_parser_expect(parser, TOKEN);                                                    \
            if (el_parser_has_errs(parser)) return el_parser_sync(parser, EL_PARSER_SYNC_DECL); \
            target = PARSE_TARGET(parser);                                                      \
            if (target == NULL) return el_parser_sync(parser, EL_PARSER_SYNC_DECL);             \
        }                                                                                       \
                                                                                                \
        APPEND_FN(&head, &tail,                                                                 \
            NEW_ELEM_FN(parser->aarena, name_tok.lexeme, target));                              \
                                                                                                \
        if (!el_parser_match(parser, EL_TT_COMMA)) break;                                       \
    }                                                                                           \
                                                                                                \
    ElToken semi_tok = el_parser_expect(parser, EL_TT_SEMICOLON);                               \
    if (el_parser_has_errs(parser)) return el_parser_sync(parser, EL_PARSER_SYNC_DECL);         \
                                                                                                \
    ElSourceSpan span = el_srcspan_merge(tok.span, semi_tok.span);                              \
    return NEW_FN(parser->aarena, span, head);                                                  \
} while (0)
// NOLINTEND(bugprone-macro-parentheses)

static ElAstDecl* parse_alias_decl(ElParser* parser, ElToken tok) {
    PARSE_GENERIC_LIST(ElAstAliasElem, ElAstToE, el_parse_toe,
            el_ast_append_alias_elem, el_ast_new_alias_elem,
            el_ast_new_alias, EL_TT_ASSIGN, false);
}

static ElAstDecl* parse_typedef_decl(ElParser* parser, ElToken tok) {
    PARSE_GENERIC_LIST(ElAstTypedefElem, ElAstType, _el_parse_type,
            el_ast_append_typedef_elem, el_ast_new_typedef_elem,
            el_ast_new_typedef, EL_TT_KW_AS, true);
}

static ElAstDecl* el_parse_internal_decl(ElParser* parser) {
    if (el_parser_check(parser, EL_TT_KW_ALIAS)) {
        ElToken alias_tok = el_parser_advance(parser);
        return parse_alias_decl(parser, alias_tok);
    }

    if (el_parser_check(parser, EL_TT_KW_TYPEDEF)) {
        ElToken typedef_tok = el_parser_advance(parser);
        return parse_typedef_decl(parser, typedef_tok);
    }

    usize idx = 0;
    if (el_parser_peek_at(parser, idx).type == EL_TT_KW_STATIC) idx++;

    if (!_el_parser_lookahead_skip_type(parser, &idx)) {
        return _el_parser_report_incomplete_decl(parser, idx);
    }
    if (el_parser_peek_at(parser, idx).type != EL_TT_IDENT) {
        return _el_parser_report_incomplete_decl(parser, idx);
    }

    if (el_parser_peek_at(parser, idx + 1).type == EL_TT_LPAREN) {
        ElAstFuncSignature sig = parse_func_sig(parser);
        if (sig.ret_type == NULL || sig.name == NULL) return el_parser_sync(parser, EL_PARSER_SYNC_DECL);
        return parse_func_internal_decl(parser, sig);
    }

    return parse_var_internal_decl(parser);
}

static ElAstDecl* _parse_decl_internal(ElParser* parser) {
    if (el_parser_check(parser, EL_TT_KW_EXTERN)) {
        ElToken extern_tok = parser->current;
        el_parser_advance(parser);
        return parse_extern_decl(parser, extern_tok);
    }

    return el_parse_internal_decl(parser);
}

ElAstDecl* el_parse_decl(ElParser* parser) {
    el_prof_begin_sub(parser->prof, parser->pss_decl);
    ElAstDecl* result = _parse_decl_internal(parser);
    el_prof_finish_sub(parser->prof, parser->pss_decl);
    return result;
}
