#include <elash/parser/parser.h>

#include <elash/diag/engine.h>
#include <elash/diag/meta.h>
#include <elash/lexer/token.h>

#include <elash/util/dynarena.h>
#include <elash/util/strconv.h>
#include <elash/util/assert.h>

#include <elash/ast/tree/expr.h>
#include <elash/ast/tree/expr/bin.h>
#include <elash/ast/tree/expr/unary.h>
#include <elash/ast/tree/expr/literal.h>
#include <elash/ast/tree/expr/cast.h>

#include <elash/ast/tree/common/ident.h>
#include <elash/ast/tree/common/init.h>

static ElAstExpr* _el_parser_parse_call(ElParser* parser, ElAstExpr* callee);

static bool _el_parser_is_type_literal(ElParser* parser) {
    usize idx = 0;
    if (!_el_parser_lookahead_skip_type(parser, &idx)) return false;
    return el_parser_peek_at(parser, idx).type == EL_TT_LBRACE;
}

// TODO: split this function into smaller helpers
//      "clang-tidy: Function '_el_parser_parse_primary' has cognitive complexity of 30 (threshold 25)" ~2026
ElAstExpr* _el_parser_parse_primary(ElParser* parser) {
    if (_el_parser_is_type_literal(parser)) {
        ElAstType* type = _el_parser_parse_type(parser);
        ElAstInit* init = el_parser_parse_init(parser);
        if (!init) return NULL;

        return el_ast_new_array_lit(parser->arena, el_source_span_merge(type->span, init->span), type, init);
    }

    if (el_parser_check(parser, EL_TT_IDENT)) {
        ElToken tok = parser->current;
        el_parser_advance(parser);

        return el_ast_new_ident(parser->arena, tok.span, tok.lexeme);
    }

    if (el_parser_check(parser, EL_TT_INT_LITERAL)) {
        ElToken tok = parser->current;
        el_parser_advance(parser);

        int64_t val = 0;
        if (!el_string_to_i64(tok.lexeme, 10, &val)) {
            el_diag_report(
                parser->diag, EL_DIAG_ERROR, "syntax.invalid-number",
                tok.span,
                "invalid integer literal"
            );
            return NULL;
        }
        return el_ast_new_int_literal(parser->arena, tok.span, val);
    }

    if (el_parser_check(parser, EL_TT_FLOAT_LITERAL)) {
        ElToken tok = parser->current;
        el_parser_advance(parser);

        long double val = 0.0L;
        if (!el_string_to_long_double(tok.lexeme, &val)) {
            el_diag_report(
                parser->diag, EL_DIAG_ERROR, "syntax.invalid-number",
                tok.span,
                "invalid float literal"
            );
            return NULL;
        }
        return el_ast_new_float_literal(parser->arena, tok.span, val);
    }

    if (el_parser_check(parser, EL_TT_STRING_LITERAL)) {
        ElToken tok = parser->current;
        el_parser_advance(parser);
        return el_ast_new_string_literal(parser->arena, tok.span, tok.lexeme);
    }

    if (el_parser_check(parser, EL_TT_CHAR_LITERAL)) {
        ElToken tok = parser->current;
        el_parser_advance(parser);

        char val = tok.lexeme.data[0];
        return el_ast_new_char_literal(parser->arena, tok.span, val);
    }

    if (el_parser_check(parser, EL_TT_TRUE_LITERAL)) {
        ElToken tok = parser->current;
        el_parser_advance(parser);
        return el_ast_new_bool_literal(parser->arena, tok.span, true);
    }

    if (el_parser_check(parser, EL_TT_FALSE_LITERAL)) {
        ElToken tok = parser->current;
        el_parser_advance(parser);
        return el_ast_new_bool_literal(parser->arena, tok.span, false);
    }

    if (el_parser_check(parser, EL_TT_NULL_LITERAL)) {
        ElToken tok = parser->current;
        el_parser_advance(parser);
        return el_ast_new_null_literal(parser->arena, tok.span);
    }

    if (el_parser_match(parser, EL_TT_LPAREN)) {
        ElAstExpr* expr = el_parser_parse_expr(parser);
        if (el_parser_has_errs(parser)) return NULL;

        if (!el_parser_check(parser, EL_TT_RPAREN)) {
            el_parser_expect(parser, EL_TT_RPAREN);
            el_parser_skip_to(parser, EL_TT_RPAREN);
            if (el_parser_check(parser, EL_TT_RPAREN)) {
                el_parser_advance(parser);
            }
        } else {
            el_parser_advance(parser);
        }

        return expr;
    }

    _el_parser_report_unexpected(parser, parser->current);
    el_parser_advance(parser);
    return NULL;
}

static ElAstExpr* _el_parser_parse_call(ElParser* parser, ElAstExpr* callee) {
    ElAstInit* args_head = NULL;
    ElAstInit* args_tail = NULL;
    usize arg_count = 0;

    if (!el_parser_check(parser, EL_TT_RPAREN)) {
        while (true) {
            ElAstInit* arg = el_parser_parse_init(parser);
            if (el_parser_has_errs(parser)) {
                el_parser_sync(parser, EL_PARSER_SYNC_EXPR);
                if (!el_parser_check(parser, EL_TT_COMMA)) {
                    break;
                }
                continue;
            }

            el_ast_init_list_append(&args_head, &args_tail, arg);
            arg_count++;

            if (!el_parser_match(parser, EL_TT_COMMA)) break;
            if (el_parser_check(parser, EL_TT_RPAREN)) break;
        }
    }

    ElToken rparen = parser->current;
    if (el_parser_check(parser, EL_TT_RPAREN)) {
        rparen = parser->current;
        el_parser_advance(parser);
    } else {
        el_parser_expect(parser, EL_TT_RPAREN);
        el_parser_skip_to(parser, EL_TT_RPAREN);
        if (el_parser_check(parser, EL_TT_RPAREN)) {
            rparen = parser->current;
            el_parser_advance(parser);
        }
    }

    return el_ast_new_call_expr(
        parser->arena,
        el_source_span_merge(callee->span, rparen.span),
        callee, args_head, arg_count
    );
}

ElAstExpr* _el_parser_parse_postfix(ElParser* parser) {
    ElAstExpr* expr = _el_parser_parse_primary(parser);
    if (el_parser_has_errs(parser)) return NULL;

    while (true) {
        if (el_parser_check(parser, EL_TT_INC)) {
            ElToken tok = parser->current;
            el_parser_advance(parser);
            expr = el_ast_new_unary_expr(parser->arena, el_source_span_merge(expr->span, tok.span), EL_SEMA_UNARY_OP_POST_INC, expr);
        } else if (el_parser_check(parser, EL_TT_DEC)) {
            ElToken tok = parser->current;
            el_parser_advance(parser);
            expr = el_ast_new_unary_expr(parser->arena, el_source_span_merge(expr->span, tok.span), EL_SEMA_UNARY_OP_POST_DEC, expr);
        } else if (el_parser_match(parser, EL_TT_LPAREN)) {
            expr = _el_parser_parse_call(parser, expr);
        } else if (el_parser_check(parser, EL_TT_CARET)) {
            ElToken tok = parser->current;
            el_parser_advance(parser);
            expr = el_ast_new_unary_expr(parser->arena, el_source_span_merge(expr->span, tok.span), EL_SEMA_UNARY_OP_DEREF, expr);
        } else if (el_parser_match(parser, EL_TT_LBRACKET)) {
            ElAstExpr* index = el_parser_parse_expr(parser);
            if (el_parser_has_errs(parser)) {
                el_parser_sync(parser, EL_PARSER_SYNC_EXPR);
            }

            ElToken rbracket = parser->current;
            if (el_parser_check(parser, EL_TT_RBRACKET)) {
                rbracket = parser->current;
                el_parser_advance(parser);
            } else {
                el_parser_expect(parser, EL_TT_RBRACKET);
                el_parser_skip_to(parser, EL_TT_RBRACKET);
                if (el_parser_check(parser, EL_TT_RBRACKET)) {
                    rbracket = parser->current;
                    el_parser_advance(parser);
                }
            }

            expr = el_ast_new_bin_expr(
                parser->arena,
                el_source_span_merge(expr->span, rbracket.span),
                EL_SEMA_BIN_OP_INDEX, expr, index
            );
        } else {
            break;
        }

        if (el_parser_has_errs(parser)) return NULL;
    }
    return expr;
}

ElAstExpr* _el_parser_parse_unary(ElParser* parser) {
    if (el_parser_check(parser, EL_TT_PLUS)) {
        ElToken tok = parser->current;
        el_parser_advance(parser);
        ElAstExpr* operand = _el_parser_parse_unary(parser);
        if (el_parser_has_errs(parser)) return NULL;
        return el_ast_new_unary_expr(parser->arena, el_source_span_merge(tok.span, operand->span), EL_SEMA_UNARY_OP_POS, operand);
    }
    if (el_parser_check(parser, EL_TT_MINUS)) {
        ElToken tok = parser->current;
        el_parser_advance(parser);
        ElAstExpr* operand = _el_parser_parse_unary(parser);
        if (el_parser_has_errs(parser)) return NULL;
        return el_ast_new_unary_expr(parser->arena, el_source_span_merge(tok.span, operand->span), EL_SEMA_UNARY_OP_NEG, operand);
    }
    if (el_parser_check(parser, EL_TT_LOGICAL_NOT)) {
        ElToken tok = parser->current;
        el_parser_advance(parser);
        ElAstExpr* operand = _el_parser_parse_unary(parser);
        if (el_parser_has_errs(parser)) return NULL;
        return el_ast_new_unary_expr(parser->arena, el_source_span_merge(tok.span, operand->span), EL_SEMA_UNARY_OP_NOT, operand);
    }
    if (el_parser_check(parser, EL_TT_BITWISE_NOT)) {
        ElToken tok = parser->current;
        el_parser_advance(parser);
        ElAstExpr* operand = _el_parser_parse_unary(parser);
        if (el_parser_has_errs(parser)) return NULL;
        return el_ast_new_unary_expr(parser->arena, el_source_span_merge(tok.span, operand->span), EL_SEMA_UNARY_OP_BW_NOT, operand);
    }
    if (el_parser_check(parser, EL_TT_INC)) {
        ElToken tok = parser->current;
        el_parser_advance(parser);
        ElAstExpr* operand = _el_parser_parse_unary(parser);
        if (el_parser_has_errs(parser)) return NULL;
        return el_ast_new_unary_expr(parser->arena, el_source_span_merge(tok.span, operand->span), EL_SEMA_UNARY_OP_PRE_INC, operand);
    }
    if (el_parser_check(parser, EL_TT_DEC)) {
        ElToken tok = parser->current;
        el_parser_advance(parser);
        ElAstExpr* operand = _el_parser_parse_unary(parser);
        if (el_parser_has_errs(parser)) return NULL;
        return el_ast_new_unary_expr(parser->arena, el_source_span_merge(tok.span, operand->span), EL_SEMA_UNARY_OP_PRE_DEC, operand);
    }
    if (el_parser_check(parser, EL_TT_BITWISE_AND)) {
        ElToken tok = parser->current;
        el_parser_advance(parser);
        ElAstExpr* operand = _el_parser_parse_unary(parser);
        if (el_parser_has_errs(parser)) return NULL;
        return el_ast_new_unary_expr(parser->arena, el_source_span_merge(tok.span, operand->span), EL_SEMA_UNARY_OP_ADDROF, operand);
    }

    return _el_parser_parse_postfix(parser);
}

ElAstExpr* _el_parser_parse_cast(ElParser* parser) {
    ElAstExpr* expr = _el_parser_parse_unary(parser);
    if (el_parser_has_errs(parser)) return NULL;

    while (el_parser_match(parser, EL_TT_KW_AS)) {
        ElAstType* type = _el_parser_parse_type(parser);
        if (type == NULL) return NULL;
        expr = el_ast_new_cast_expr(parser->arena, el_source_span_merge(expr->span, type->span), expr, type);
    }
    return expr;
}

ElAstExpr* _el_parser_parse_multiplicative(ElParser* parser) {
    ElAstExpr* expr = _el_parser_parse_cast(parser);
    if (el_parser_has_errs(parser)) return NULL;

    while (true) {
        ElAstBinOp type;
        if (el_parser_match(parser, EL_TT_STAR)) type = EL_SEMA_BIN_OP_MUL;
        else if (el_parser_match(parser, EL_TT_SLASH)) type = EL_SEMA_BIN_OP_DIV;
        else if (el_parser_match(parser, EL_TT_PERCENT)) type = EL_SEMA_BIN_OP_MOD;
        else break;

        ElAstExpr* right = _el_parser_parse_cast(parser);
        if (el_parser_has_errs(parser)) {
            el_parser_sync(parser, EL_PARSER_SYNC_EXPR);
            break;
        }
        expr = el_ast_new_bin_expr(parser->arena, el_source_span_merge(expr->span, right->span), type, expr, right);
    }
    return expr;
}

ElAstExpr* _el_parser_parse_additive(ElParser* parser) {
    ElAstExpr* expr = _el_parser_parse_multiplicative(parser);
    if (el_parser_has_errs(parser)) return NULL;

    while (true) {
        ElAstBinOp type;
        if (el_parser_match(parser, EL_TT_PLUS)) type = EL_SEMA_BIN_OP_ADD;
        else if (el_parser_match(parser, EL_TT_MINUS)) type = EL_SEMA_BIN_OP_SUB;
        else break;

        ElAstExpr* right = _el_parser_parse_multiplicative(parser);
        if (el_parser_has_errs(parser)) {
            el_parser_sync(parser, EL_PARSER_SYNC_EXPR);
            break;
        }
        expr = el_ast_new_bin_expr(parser->arena, el_source_span_merge(expr->span, right->span), type, expr, right);
    }
    return expr;
}

ElAstExpr* _el_parser_parse_shift(ElParser* parser) {
    ElAstExpr* expr = _el_parser_parse_additive(parser);
    if (el_parser_has_errs(parser)) return NULL;

    while (true) {
        ElAstBinOp type;
        if (el_parser_match(parser, EL_TT_SHL)) type = EL_SEMA_BIN_OP_SHL;
        else if (el_parser_match(parser, EL_TT_SHR)) type = EL_SEMA_BIN_OP_SHR;
        else break;

        ElAstExpr* right = _el_parser_parse_additive(parser);
        if (el_parser_has_errs(parser)) {
            el_parser_sync(parser, EL_PARSER_SYNC_EXPR);
            break;
        }
        expr = el_ast_new_bin_expr(parser->arena, el_source_span_merge(expr->span, right->span), type, expr, right);
    }
    return expr;
}

ElAstExpr* _el_parser_parse_relational(ElParser* parser) {
    ElAstExpr* expr = _el_parser_parse_shift(parser);
    if (el_parser_has_errs(parser)) return NULL;

    while (true) {
        ElAstBinOp type;
        if (el_parser_match(parser, EL_TT_LT)) type = EL_SEMA_BIN_OP_LT;
        else if (el_parser_match(parser, EL_TT_LTE)) type = EL_SEMA_BIN_OP_LTE;
        else if (el_parser_match(parser, EL_TT_GT)) type = EL_SEMA_BIN_OP_GT;
        else if (el_parser_match(parser, EL_TT_GTE)) type = EL_SEMA_BIN_OP_GTE;
        else break;

        ElAstExpr* right = _el_parser_parse_shift(parser);
        if (el_parser_has_errs(parser)) {
            el_parser_sync(parser, EL_PARSER_SYNC_EXPR);
            break;
        }
        expr = el_ast_new_bin_expr(parser->arena, el_source_span_merge(expr->span, right->span), type, expr, right);
    }
    return expr;
}

ElAstExpr* _el_parser_parse_equality(ElParser* parser) {
    ElAstExpr* expr = _el_parser_parse_relational(parser);
    if (el_parser_has_errs(parser)) return NULL;

    while (true) {
        ElAstBinOp type;
        if (el_parser_match(parser, EL_TT_EQL)) type = EL_SEMA_BIN_OP_EQ;
        else if (el_parser_match(parser, EL_TT_NEQ)) type = EL_SEMA_BIN_OP_NEQ;
        else break;

        ElAstExpr* right = _el_parser_parse_relational(parser);
        if (el_parser_has_errs(parser)) {
            el_parser_sync(parser, EL_PARSER_SYNC_EXPR);
            break;
        }
        expr = el_ast_new_bin_expr(parser->arena, el_source_span_merge(expr->span, right->span), type, expr, right);
    }
    return expr;
}

ElAstExpr* _el_parser_parse_bitwise_and(ElParser* parser) {
    ElAstExpr* expr = _el_parser_parse_equality(parser);
    if (el_parser_has_errs(parser)) return NULL;

    while (el_parser_match(parser, EL_TT_BITWISE_AND)) {
        ElAstExpr* right = _el_parser_parse_equality(parser);
        if (el_parser_has_errs(parser)) {
            el_parser_sync(parser, EL_PARSER_SYNC_EXPR);
            break;
        }
        expr = el_ast_new_bin_expr(parser->arena, el_source_span_merge(expr->span, right->span), EL_SEMA_BIN_OP_BW_AND, expr, right);
    }
    return expr;
}

ElAstExpr* _el_parser_parse_bitwise_xor(ElParser* parser) {
    ElAstExpr* expr = _el_parser_parse_bitwise_and(parser);
    if (el_parser_has_errs(parser)) return NULL;

    while (el_parser_match(parser, EL_TT_BITWISE_XOR)) {
        ElAstExpr* right = _el_parser_parse_bitwise_and(parser);
        if (el_parser_has_errs(parser)) {
            el_parser_sync(parser, EL_PARSER_SYNC_EXPR);
            break;
        }
        expr = el_ast_new_bin_expr(parser->arena, el_source_span_merge(expr->span, right->span), EL_SEMA_BIN_OP_BW_XOR, expr, right);
    }
    return expr;
}

ElAstExpr* _el_parser_parse_bitwise_or(ElParser* parser) {
    ElAstExpr* expr = _el_parser_parse_bitwise_xor(parser);
    if (el_parser_has_errs(parser)) return NULL;

    while (el_parser_match(parser, EL_TT_BITWISE_OR)) {
        ElAstExpr* right = _el_parser_parse_bitwise_xor(parser);
        if (el_parser_has_errs(parser)) {
            el_parser_sync(parser, EL_PARSER_SYNC_EXPR);
            break;
        }
        expr = el_ast_new_bin_expr(parser->arena, el_source_span_merge(expr->span, right->span), EL_SEMA_BIN_OP_BW_OR, expr, right);
    }
    return expr;
}

ElAstExpr* _el_parser_parse_bitwise_imp(ElParser* parser) {
    ElAstExpr* expr = _el_parser_parse_bitwise_or(parser);
    if (el_parser_has_errs(parser)) return NULL;

    while (el_parser_match(parser, EL_TT_BITWISE_IMP)) {
        ElAstExpr* right = _el_parser_parse_bitwise_or(parser);
        if (el_parser_has_errs(parser)) {
            el_parser_sync(parser, EL_PARSER_SYNC_EXPR);
            break;
        }
        expr = el_ast_new_bin_expr(parser->arena, el_source_span_merge(expr->span, right->span), EL_SEMA_BIN_OP_BW_IMP, expr, right);
    }
    return expr;
}

ElAstExpr* _el_parser_parse_logical_and(ElParser* parser) {
    ElAstExpr* expr = _el_parser_parse_bitwise_imp(parser);
    if (el_parser_has_errs(parser)) return NULL;

    while (el_parser_match(parser, EL_TT_LOGICAL_AND)) {
        ElAstExpr* right = _el_parser_parse_bitwise_imp(parser);
        if (el_parser_has_errs(parser)) {
            el_parser_sync(parser, EL_PARSER_SYNC_EXPR);
            break;
        }
        expr = el_ast_new_bin_expr(parser->arena, el_source_span_merge(expr->span, right->span), EL_SEMA_BIN_OP_AND, expr, right);
    }
    return expr;
}

ElAstExpr* _el_parser_parse_logical_or(ElParser* parser) {
    ElAstExpr* expr = _el_parser_parse_logical_and(parser);
    if (el_parser_has_errs(parser)) return NULL;

    while (el_parser_match(parser, EL_TT_LOGICAL_OR)) {
        ElAstExpr* right = _el_parser_parse_logical_and(parser);
        if (el_parser_has_errs(parser)) {
            el_parser_sync(parser, EL_PARSER_SYNC_EXPR);
            break;
        }
        expr = el_ast_new_bin_expr(parser->arena, el_source_span_merge(expr->span, right->span), EL_SEMA_BIN_OP_OR, expr, right);
    }
    return expr;
}

ElAstExpr* _el_parser_parse_logical_imp(ElParser* parser) {
    ElAstExpr* expr = _el_parser_parse_logical_or(parser);
    if (el_parser_has_errs(parser)) return NULL;

    while (el_parser_match(parser, EL_TT_LOGICAL_IMP)) {
        ElAstExpr* right = _el_parser_parse_logical_or(parser);
        if (el_parser_has_errs(parser)) {
            el_parser_sync(parser, EL_PARSER_SYNC_EXPR);
            break;
        }
        expr = el_ast_new_bin_expr(parser->arena, el_source_span_merge(expr->span, right->span), EL_SEMA_BIN_OP_IMP, expr, right);
    }
    return expr;
}

ElAstExpr* el_parser_parse_expr(ElParser* parser) {
    return _el_parser_parse_logical_imp(parser);
}
