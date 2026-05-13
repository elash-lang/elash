#include <elash/parser/parser.h>
#include <elash/parser/utility.h>

#include <elash/util/dynarena.h>
#include <elash/util/strconv.h>

#include <elash/ast/tree/expr.h>
#include <elash/ast/tree/expr/bin.h>
#include <elash/ast/tree/expr/unary.h>
#include <elash/ast/tree/expr/literal.h>

#include <elash/ast/tree/common/ident.h>

ElParserErrorCode _el_parser_parse_primary(ElParser* parser, ElAstExprNode** out) {
    if (el_parser_check(parser, EL_TT_IDENT)) {
        ElToken tok = parser->current;
        el_parser_advance(parser);

        *out = el_ast_new_ident_node(parser->arena, tok.span, tok.lexeme);
        return _el_parser_ret_ok(parser);
    }

    if (el_parser_check(parser, EL_TT_INT_LITERAL)) {
        ElToken tok = parser->current;
        el_parser_advance(parser);

        int64_t val = 0;
        if (!el_string_to_i64(tok.lexeme, 10, &val)) {
            return _el_parser_ret_err(parser, .code = EL_PARSER_ERR_UNEXPECTED_TOKEN, .token = tok);
        }
        *out = el_ast_new_int_literal(parser->arena, tok.span, val);
        return _el_parser_ret_ok(parser);
    }

    if (el_parser_check(parser, EL_TT_FLOAT_LITERAL)) {
        ElToken tok = parser->current;
        el_parser_advance(parser);

        long double val = 0.0L;
        if (!el_string_to_long_double(tok.lexeme, &val)) {
            return _el_parser_ret_err(parser, .code = EL_PARSER_ERR_UNEXPECTED_TOKEN, .token = tok);
        }
        *out = el_ast_new_float_literal(parser->arena, tok.span, val);
        return _el_parser_ret_ok(parser);
    }

    if (el_parser_check(parser, EL_TT_STRING_LITERAL)) {
        ElToken tok = parser->current;
        el_parser_advance(parser);
        *out = el_ast_new_string_literal(parser->arena, tok.span, tok.lexeme);
        return _el_parser_ret_ok(parser);
    }

    if (el_parser_check(parser, EL_TT_CHAR_LITERAL)) {
        ElToken tok = parser->current;
        el_parser_advance(parser);

        char val = 0;
        if (tok.lexeme.len >= 3) {
             val = tok.lexeme.data[1];
        }
        *out = el_ast_new_char_literal(parser->arena, tok.span, val);
        return _el_parser_ret_ok(parser);
    }

    if (el_parser_check(parser, EL_TT_TRUE_LITERAL)) {
        ElToken tok = parser->current;
        el_parser_advance(parser);
        *out = el_ast_new_bool_literal(parser->arena, tok.span, true);
        return _el_parser_ret_ok(parser);
    }

    if (el_parser_check(parser, EL_TT_FALSE_LITERAL)) {
        ElToken tok = parser->current;
        el_parser_advance(parser);
        *out = el_ast_new_bool_literal(parser->arena, tok.span, false);
        return _el_parser_ret_ok(parser);
    }

    if (el_parser_check(parser, EL_TT_NULL_LITERAL)) {
        ElToken tok = parser->current;
        el_parser_advance(parser);
        *out = el_ast_new_null_literal(parser->arena, tok.span);
        return _el_parser_ret_ok(parser);
    }

    if (el_parser_match(parser, EL_TT_LPAREN)) {
        ElParserErrorCode err = _el_parser_parse_expression(parser, out);
        if (err != EL_PARSER_ERR_OK) return err;
        return el_parser_expect(parser, EL_TT_RPAREN);
    }

    return _el_parser_ret_err(parser, .code = EL_PARSER_ERR_UNEXPECTED_TOKEN, .token = parser->current);
}

ElParserErrorCode _el_parser_parse_call(ElParser* parser, ElAstExprNode** out) {
    ElAstExprNode* args_head = NULL;
    ElAstExprNode* args_tail = NULL;
    usize arg_count = 0;

    if (!el_parser_check(parser, EL_TT_RPAREN)) {
        while (true) {
            ElAstExprNode* arg = NULL;
            ElParserErrorCode err = _el_parser_parse_expression(parser, &arg);
            if (err != EL_PARSER_ERR_OK) return err;

            el_ast_expr_list_append(&args_head, &args_tail, arg);
            arg_count++;

            if (!el_parser_match(parser, EL_TT_COMMA)) break;
        };
    }

    ElToken rparen = parser->current;
    ElParserErrorCode err = el_parser_expect(parser, EL_TT_RPAREN);
    if (err != EL_PARSER_ERR_OK) return err;

    *out = el_ast_new_call_expr(
        parser->arena,
        el_source_span_merge((*out)->span, rparen.span),
        *out, args_head, arg_count
    );
    return _el_parser_ret_ok(parser);
}

ElParserErrorCode _el_parser_parse_postfix(ElParser* parser, ElAstExprNode** out) {
    ElParserErrorCode err = _el_parser_parse_primary(parser, out);
    if (err != EL_PARSER_ERR_OK) return err;

    while (true) {
        if (el_parser_check(parser, EL_TT_INC)) {
            ElToken tok = parser->current;
            el_parser_advance(parser);
            *out = el_ast_new_unary_expr(parser->arena, el_source_span_merge((*out)->span, tok.span), EL_SEMA_UNARY_OP_POST_INC, *out);
        } else if (el_parser_check(parser, EL_TT_DEC)) {
            ElToken tok = parser->current;
            el_parser_advance(parser);
            *out = el_ast_new_unary_expr(parser->arena, el_source_span_merge((*out)->span, tok.span), EL_SEMA_UNARY_OP_POST_DEC, *out);
        } else if (el_parser_match(parser, EL_TT_LPAREN)) {
            return _el_parser_parse_call(parser, out);
        } else {
            break;
        }
    }
    return _el_parser_ret_ok(parser);
}

ElParserErrorCode _el_parser_parse_unary(ElParser* parser, ElAstExprNode** out) {
    if (el_parser_check(parser, EL_TT_PLUS)) {
        ElToken tok = parser->current;
        el_parser_advance(parser);
        ElParserErrorCode err = _el_parser_parse_unary(parser, out);
        if (err != EL_PARSER_ERR_OK) return err;
        *out = el_ast_new_unary_expr(parser->arena, el_source_span_merge(tok.span, (*out)->span), EL_SEMA_UNARY_OP_POS, *out);
        return _el_parser_ret_ok(parser);
    }
    if (el_parser_check(parser, EL_TT_MINUS)) {
        ElToken tok = parser->current;
        el_parser_advance(parser);
        ElParserErrorCode err = _el_parser_parse_unary(parser, out);
        if (err != EL_PARSER_ERR_OK) return err;
        *out = el_ast_new_unary_expr(parser->arena, el_source_span_merge(tok.span, (*out)->span), EL_SEMA_UNARY_OP_NEG, *out);
        return _el_parser_ret_ok(parser);
    }
    if (el_parser_check(parser, EL_TT_LOGICAL_NOT)) {
        ElToken tok = parser->current;
        el_parser_advance(parser);
        ElParserErrorCode err = _el_parser_parse_unary(parser, out);
        if (err != EL_PARSER_ERR_OK) return err;
        *out = el_ast_new_unary_expr(parser->arena, el_source_span_merge(tok.span, (*out)->span), EL_SEMA_UNARY_OP_NOT, *out);
        return _el_parser_ret_ok(parser);
    }
    if (el_parser_check(parser, EL_TT_BITWISE_NOT)) {
        ElToken tok = parser->current;
        el_parser_advance(parser);
        ElParserErrorCode err = _el_parser_parse_unary(parser, out);
        if (err != EL_PARSER_ERR_OK) return err;
        *out = el_ast_new_unary_expr(parser->arena, el_source_span_merge(tok.span, (*out)->span), EL_SEMA_UNARY_OP_BW_NOT, *out);
        return _el_parser_ret_ok(parser);
    }
    if (el_parser_check(parser, EL_TT_INC)) {
        ElToken tok = parser->current;
        el_parser_advance(parser);
        ElParserErrorCode err = _el_parser_parse_unary(parser, out);
        if (err != EL_PARSER_ERR_OK) return err;
        *out = el_ast_new_unary_expr(parser->arena, el_source_span_merge(tok.span, (*out)->span), EL_SEMA_UNARY_OP_PRE_INC, *out);
        return _el_parser_ret_ok(parser);
    }
    if (el_parser_check(parser, EL_TT_DEC)) {
        ElToken tok = parser->current;
        el_parser_advance(parser);
        ElParserErrorCode err = _el_parser_parse_unary(parser, out);
        if (err != EL_PARSER_ERR_OK) return err;
        *out = el_ast_new_unary_expr(parser->arena, el_source_span_merge(tok.span, (*out)->span), EL_SEMA_UNARY_OP_PRE_DEC, *out);
        return _el_parser_ret_ok(parser);
    }

    return _el_parser_parse_postfix(parser, out);
}

ElParserErrorCode _el_parser_parse_multiplicative(ElParser* parser, ElAstExprNode** out) {
    ElParserErrorCode err = _el_parser_parse_unary(parser, out);
    if (err != EL_PARSER_ERR_OK) return err;

    while (true) {
        ElAstBinOp type;
        if (el_parser_match(parser, EL_TT_STAR)) type = EL_SEMA_BIN_OP_MUL;
        else if (el_parser_match(parser, EL_TT_SLASH)) type = EL_SEMA_BIN_OP_DIV;
        else if (el_parser_match(parser, EL_TT_PERCENT)) type = EL_SEMA_BIN_OP_MOD;
        else break;

        ElAstExprNode* right = NULL;
        err = _el_parser_parse_unary(parser, &right);
        if (err != EL_PARSER_ERR_OK) return err;
        *out = el_ast_new_bin_expr(parser->arena, el_source_span_merge((*out)->span, right->span), type, *out, right);
    }
    return _el_parser_ret_ok(parser);
}

ElParserErrorCode _el_parser_parse_additive(ElParser* parser, ElAstExprNode** out) {
    ElParserErrorCode err = _el_parser_parse_multiplicative(parser, out);
    if (err != EL_PARSER_ERR_OK) return err;

    while (true) {
        ElAstBinOp type;
        if (el_parser_match(parser, EL_TT_PLUS)) type = EL_SEMA_BIN_OP_ADD;
        else if (el_parser_match(parser, EL_TT_MINUS)) type = EL_SEMA_BIN_OP_SUB;
        else break;

        ElAstExprNode* right = NULL;
        err = _el_parser_parse_multiplicative(parser, &right);
        if (err != EL_PARSER_ERR_OK) return err;
        *out = el_ast_new_bin_expr(parser->arena, el_source_span_merge((*out)->span, right->span), type, *out, right);
    }
    return _el_parser_ret_ok(parser);
}

ElParserErrorCode _el_parser_parse_shift(ElParser* parser, ElAstExprNode** out) {
    ElParserErrorCode err = _el_parser_parse_additive(parser, out);
    if (err != EL_PARSER_ERR_OK) return err;

    while (true) {
        ElAstBinOp type;
        if (el_parser_match(parser, EL_TT_SHL)) type = EL_SEMA_BIN_OP_SHL;
        else if (el_parser_match(parser, EL_TT_SHR)) type = EL_SEMA_BIN_OP_SHR;
        else break;

        ElAstExprNode* right = NULL;
        err = _el_parser_parse_additive(parser, &right);
        if (err != EL_PARSER_ERR_OK) return err;
        *out = el_ast_new_bin_expr(parser->arena, el_source_span_merge((*out)->span, right->span), type, *out, right);
    }
    return _el_parser_ret_ok(parser);
}

ElParserErrorCode _el_parser_parse_relational(ElParser* parser, ElAstExprNode** out) {
    ElParserErrorCode err = _el_parser_parse_shift(parser, out);
    if (err != EL_PARSER_ERR_OK) return err;

    while (true) {
        ElAstBinOp type;
        if (el_parser_match(parser, EL_TT_LT)) type = EL_SEMA_BIN_OP_LT;
        else if (el_parser_match(parser, EL_TT_LTE)) type = EL_SEMA_BIN_OP_LTE;
        else if (el_parser_match(parser, EL_TT_GT)) type = EL_SEMA_BIN_OP_GT;
        else if (el_parser_match(parser, EL_TT_GTE)) type = EL_SEMA_BIN_OP_GTE;
        else break;

        ElAstExprNode* right = NULL;
        err = _el_parser_parse_shift(parser, &right);
        if (err != EL_PARSER_ERR_OK) return err;
        *out = el_ast_new_bin_expr(parser->arena, el_source_span_merge((*out)->span, right->span), type, *out, right);
    }
    return _el_parser_ret_ok(parser);
}

ElParserErrorCode _el_parser_parse_equality(ElParser* parser, ElAstExprNode** out) {
    ElParserErrorCode err = _el_parser_parse_relational(parser, out);
    if (err != EL_PARSER_ERR_OK) return err;

    while (true) {
        ElAstBinOp type;
        if (el_parser_match(parser, EL_TT_EQL)) type = EL_SEMA_BIN_OP_EQ;
        else if (el_parser_match(parser, EL_TT_NEQ)) type = EL_SEMA_BIN_OP_NEQ;
        else break;

        ElAstExprNode* right = NULL;
        err = _el_parser_parse_relational(parser, &right);
        if (err != EL_PARSER_ERR_OK) return err;
        *out = el_ast_new_bin_expr(parser->arena, el_source_span_merge((*out)->span, right->span), type, *out, right);
    }
    return _el_parser_ret_ok(parser);
}

ElParserErrorCode _el_parser_parse_bitwise_and(ElParser* parser, ElAstExprNode** out) {
    ElParserErrorCode err = _el_parser_parse_equality(parser, out);
    if (err != EL_PARSER_ERR_OK) return err;

    while (el_parser_match(parser, EL_TT_BITWISE_AND)) {
        ElAstExprNode* right = NULL;
        err = _el_parser_parse_equality(parser, &right);
        if (err != EL_PARSER_ERR_OK) return err;
        *out = el_ast_new_bin_expr(parser->arena, el_source_span_merge((*out)->span, right->span), EL_SEMA_BIN_OP_BW_AND, *out, right);
    }
    return _el_parser_ret_ok(parser);
}

ElParserErrorCode _el_parser_parse_bitwise_xor(ElParser* parser, ElAstExprNode** out) {
    ElParserErrorCode err = _el_parser_parse_bitwise_and(parser, out);
    if (err != EL_PARSER_ERR_OK) return err;

    while (el_parser_match(parser, EL_TT_BITWISE_XOR)) {
        ElAstExprNode* right = NULL;
        err = _el_parser_parse_bitwise_and(parser, &right);
        if (err != EL_PARSER_ERR_OK) return err;
        *out = el_ast_new_bin_expr(parser->arena, el_source_span_merge((*out)->span, right->span), EL_SEMA_BIN_OP_BW_XOR, *out, right);
    }
    return _el_parser_ret_ok(parser);
}

ElParserErrorCode _el_parser_parse_bitwise_or(ElParser* parser, ElAstExprNode** out) {
    ElParserErrorCode err = _el_parser_parse_bitwise_xor(parser, out);
    if (err != EL_PARSER_ERR_OK) return err;

    while (el_parser_match(parser, EL_TT_BITWISE_OR)) {
        ElAstExprNode* right = NULL;
        err = _el_parser_parse_bitwise_xor(parser, &right);
        if (err != EL_PARSER_ERR_OK) return err;
        *out = el_ast_new_bin_expr(parser->arena, el_source_span_merge((*out)->span, right->span), EL_SEMA_BIN_OP_BW_OR, *out, right);
    }
    return _el_parser_ret_ok(parser);
}

ElParserErrorCode _el_parser_parse_logical_and(ElParser* parser, ElAstExprNode** out) {
    ElParserErrorCode err = _el_parser_parse_bitwise_or(parser, out);
    if (err != EL_PARSER_ERR_OK) return err;

    while (el_parser_match(parser, EL_TT_LOGICAL_AND)) {
        ElAstExprNode* right = NULL;
        err = _el_parser_parse_bitwise_or(parser, &right);
        if (err != EL_PARSER_ERR_OK) return err;
        *out = el_ast_new_bin_expr(parser->arena, el_source_span_merge((*out)->span, right->span), EL_SEMA_BIN_OP_AND, *out, right);
    }
    return _el_parser_ret_ok(parser);
}

ElParserErrorCode _el_parser_parse_logical_or(ElParser* parser, ElAstExprNode** out) {
    ElParserErrorCode err = _el_parser_parse_logical_and(parser, out);
    if (err != EL_PARSER_ERR_OK) return err;

    while (el_parser_match(parser, EL_TT_LOGICAL_OR)) {
        ElAstExprNode* right = NULL;
        err = _el_parser_parse_logical_and(parser, &right);
        if (err != EL_PARSER_ERR_OK) return err;
        *out = el_ast_new_bin_expr(parser->arena, el_source_span_merge((*out)->span, right->span), EL_SEMA_BIN_OP_OR, *out, right);
    }
    return _el_parser_ret_ok(parser);
}

ElParserErrorCode _el_parser_parse_expression(ElParser* parser, ElAstExprNode** out) {
    return _el_parser_parse_logical_or(parser, out);
}
