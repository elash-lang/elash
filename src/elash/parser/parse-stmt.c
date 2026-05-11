#include <elash/parser/parser.h>
#include <elash/parser/utility.h>

#include <elash/ast/stmt.h>
#include <elash/ast/stmt/return.h>

ElParserErrorCode _el_parser_parse_return(ElParser* parser, ElToken return_tok, ElAstStmtNode** out) {
    if (el_parser_check(parser, EL_TT_SEMICOLON)) {
        ElToken semi_tok = parser->current;
        el_parser_advance(parser);
        *out = el_ast_new_return_stmt(parser->arena, el_source_span_merge(return_tok.span, semi_tok.span), NULL);
        return _el_parser_ret_ok(parser);
    }

    ElAstExprNode* expr;
    ElParserErrorCode result = _el_parser_parse_expression(parser, &expr);
    if (result != EL_PARSER_ERR_OK) {
        return result;
    }

    ElToken semi_tok = parser->current;
    result = el_parser_expect(parser, EL_TT_SEMICOLON);
    if (result != EL_PARSER_ERR_OK) return result;

    *out = el_ast_new_return_stmt(parser->arena, el_source_span_merge(return_tok.span, semi_tok.span), expr);
    return _el_parser_ret_ok(parser);
}

ElParserErrorCode _el_parser_parse_expr_stmt(ElParser* parser, ElAstStmtNode** out) {
    ElAstExprNode* expr;
    ElParserErrorCode result = _el_parser_parse_expression(parser, &expr);
    if (result != EL_PARSER_ERR_OK) {
        return result;
    }

    ElToken semi_tok = parser->current;
    result = el_parser_expect(parser, EL_TT_SEMICOLON);
    if (result != EL_PARSER_ERR_OK) return result;

    *out = el_ast_new_expr_stmt(parser->arena, el_source_span_merge(expr->span, semi_tok.span), expr);
    return _el_parser_ret_ok(parser);
}

ElParserErrorCode _el_parser_parse_block(ElParser* parser, ElToken lbrace_tok, ElAstStmtNode** out) {
    ElAstStmtNode* head = NULL;
    ElAstStmtNode* tail = NULL;

    while (parser->current.type != EL_TT_RBRACE && parser->current.type != EL_TT_EOF) {
        ElAstStmtNode* stmt;
        ElParserErrorCode result = _el_parser_parse_stmt(parser, &stmt);
        if (result != EL_PARSER_ERR_OK) {
            return result;
        }

        el_ast_stmt_list_append(&head, &tail, stmt);
    }

    ElToken rbrace_tok = parser->current;
    ElParserErrorCode result = el_parser_expect(parser, EL_TT_RBRACE);
    if (result != EL_PARSER_ERR_OK) {
        return result;
    }

    *out = el_ast_new_block_stmt(parser->arena, el_source_span_merge(lbrace_tok.span, rbrace_tok.span), head);
    return _el_parser_ret_ok(parser);
}

ElParserErrorCode _el_parser_parse_var_def(ElParser* parser, ElAstStmtNode** out) {
    ElParserErrorCode result;
    ElAstTypeNode* type;
    result = _el_parser_parse_type(parser, &type);
    if (result != EL_PARSER_ERR_OK) return result;

    ElAstIdentNode* name;
    result = _el_parser_parse_ident(parser, &name);
    if (result != EL_PARSER_ERR_OK) return result;

    ElAstExprNode* init = NULL;
    if (el_parser_match(parser, EL_TT_ASSIGN)) {
        result = _el_parser_parse_expression(parser, &init);
        if (result != EL_PARSER_ERR_OK) return result;
    }

    ElToken semi_tok = parser->current;
    result = el_parser_expect(parser, EL_TT_SEMICOLON);
    if (result != EL_PARSER_ERR_OK) return result;

    *out = el_ast_new_var_def_stmt(parser->arena, el_source_span_merge(type->span, semi_tok.span), type, name, init);
    return _el_parser_ret_ok(parser);
}

ElParserErrorCode _el_parser_parse_stmt(ElParser* parser, ElAstStmtNode** out) {
    if (el_parser_check(parser, EL_TT_KW_RETURN)) {
        ElToken return_tok = parser->current;
        el_parser_advance(parser);
        return _el_parser_parse_return(parser, return_tok, out);
    }
    if (el_parser_check(parser, EL_TT_LBRACE)) {
        ElToken lbrace_tok = parser->current;
        el_parser_advance(parser);
        return _el_parser_parse_block(parser, lbrace_tok, out);
    }

    if (el_parser_check(parser, EL_TT_IDENT)) {
        ElTokenType next_type = el_parser_peek(parser).type;
        if (next_type == EL_TT_IDENT || next_type == EL_TT_STAR) {
            return _el_parser_parse_var_def(parser, out);
        }
    }

    return _el_parser_parse_expr_stmt(parser, out);
}
