#include <elash/parser/parser.h>

#include <elash/diag/engine.h>
#include <elash/diag/meta.h>
#include <elash/lexer/token.h>

#include <elash/ast/tree/toplevel.h>
#include <elash/ast/tree/toplevel/func-def.h>
#include <elash/ast/tree/toplevel/func-decl.h>

static ElAstFuncParamList _el_parser_parse_func_params(ElParser* parser) {
    ElAstFuncParamList params = el_ast_make_func_param_list();

    while (parser->current.type != EL_TT_RPAREN) {
        ElAstType* p_type = _el_parser_parse_type(parser);
        if (el_parser_has_errs(parser)) {
            el_parser_skip_to(parser, EL_TT_RPAREN);
            return params;
        }

        ElAstIdent* p_name = _el_parser_parse_ident(parser);
        if (el_parser_has_errs(parser)) {
            el_parser_skip_to(parser, EL_TT_RPAREN);
            return params;
        }

        el_ast_func_param_list_append(&params, el_ast_new_func_param(
            parser->arena,
            el_source_span_merge(p_type->span, p_name->span),
            p_type, p_name
        ));

        if (parser->current.type == EL_TT_COMMA) {
            el_parser_advance(parser);
        } else if (parser->current.type != EL_TT_RPAREN) {
            el_diag_report(
                parser->diag, EL_DIAG_ERROR, "syntax.expected-token",
                parser->current.span,
                "expected ${expected}, found ${found}",
                EL_DIAG_STRING("expected", el_token_type_to_string(EL_TT_RPAREN)),
                EL_DIAG_STRING("found", el_token_type_to_string(parser->current.type))
            );
            return params;
        }
    }

    return params;
}

static ElAstFuncSignature _el_parser_parse_func_sig(ElParser* parser) {
    ElAstFuncSignature sig = {0};
    uint errs_before = el_parser_error_count(parser);

    ElAstType* ret_type = _el_parser_parse_type(parser);
    if (el_parser_had_new_errors(parser, errs_before)) return sig;

    ElAstIdent* name = _el_parser_parse_ident(parser);
    if (el_parser_had_new_errors(parser, errs_before)) return sig;

    el_parser_expect(parser, EL_TT_LPAREN);
    if (el_parser_had_new_errors(parser, errs_before)) return sig;

    ElAstFuncParamList params = _el_parser_parse_func_params(parser);
    if (el_parser_had_new_errors(parser, errs_before)) return sig;

    ElToken rparen_tok = parser->current;
    if (!el_parser_check(parser, EL_TT_RPAREN)) {
        el_parser_expect(parser, EL_TT_RPAREN);
        return sig;
    }
    el_parser_advance(parser);

    if (ret_type == NULL || name == NULL) {
        return sig;
    }

    ElSourceSpan span = el_source_span_merge(ret_type->span, rparen_tok.span);
    return el_ast_func_signature(span, ret_type, name, params);
}

ElAstTopLevel* el_parser_parse_toplevel(ElParser* parser) {
    bool is_extern = false;
    ElToken extern_tok;
    if (el_parser_check(parser, EL_TT_KW_EXTERN)) {
        extern_tok = parser->current;
        el_parser_advance(parser);
        is_extern = true;
    }

    uint errs_before = el_parser_error_count(parser);
    ElAstFuncSignature sig = _el_parser_parse_func_sig(parser);
    if (el_parser_had_new_errors(parser, errs_before) || sig.ret_type == NULL || sig.name == NULL) {
        if (el_parser_check(parser, EL_TT_LBRACE)) {
            el_parser_skip_balanced_braces(parser);
            return NULL;
        }

        return el_parser_sync(parser, EL_PARSER_SYNC_DECL);
    }

    if (el_parser_check(parser, EL_TT_LBRACE)) {
        if (is_extern) {
            el_diag_report(
                parser->diag, EL_DIAG_ERROR, "syntax.unexpected-token",
                extern_tok.span,
                "extern declaration cannot have a body"
            );
            el_parser_skip_balanced_braces(parser);
            return NULL;
        }

        ElToken lbrace_tok = parser->current;
        el_parser_advance(parser);

        ElAstStmt* body_stmt = _el_parser_parse_block(parser, lbrace_tok);

        ElSourceSpan span = el_source_span_merge(sig.span, body_stmt->span);
        return el_ast_new_func_def(parser->arena, span, sig, &body_stmt->as.block);
    }

    ElToken semi_tok = parser->current;
    if (!el_parser_check(parser, EL_TT_SEMICOLON)) {
        el_parser_expect(parser, EL_TT_SEMICOLON);
        return el_parser_sync(parser, EL_PARSER_SYNC_DECL);
    }
    el_parser_advance(parser);

    ElSourceSpan span = sig.span;
    if (is_extern) {
        span = el_source_span_merge(extern_tok.span, span);
    }
    span = el_source_span_merge(span, semi_tok.span);

    return el_ast_new_func_decl(parser->arena, span, sig);
}
