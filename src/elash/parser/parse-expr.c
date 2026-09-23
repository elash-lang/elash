#include "parser-internals.h"

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
#include <elash/ast/tree/init.h>

#include <elash/sema/intparse.h>
#include <elash/sema/strparse.h>

bool _el_parser_is_type_literal(ElParser* parser) {
    usize idx = 0;
    if (el_parser_peek_at(parser, idx).type == EL_TT_KW_STATIC) idx++;
    if (!_el_parser_lookahead_skip_type(parser, &idx)) return false;
    return el_parser_peek_at(parser, idx).type == EL_TT_LBRACE;
}

static ElAstExpr* parse_string_lit(ElParser* parser) {
    ElSourceSpan combined_span = el_parser_peek(parser).span;

    usize total_cap = 0;
    usize count = 0;

    ElToken tok;
    while ((tok = el_parser_peek_at(parser, count)).type == EL_TT_STRING_LITERAL) {
        total_cap += tok.lexeme.len;
        combined_span = el_srcspan_merge(combined_span, tok.span);
        count++;
    }

    char* buf = EL_DYNARENA_NEW_ARR(parser->farena, char, total_cap);
    usize current_len = 0;

    for (usize i = 0; i < count; i++) {
        ElToken tok = el_parser_advance(parser);

        ElStringView str = el_parse_str_with_escapes(parser->diag, tok, buf + current_len);
        if (el_sv_is_null(str)) return NULL;

        current_len += str.len;
    }

    ElStringView total_str = el_sv_from_data_and_len(buf, current_len);
    return el_ast_new_str_lit(parser->aarena, combined_span, total_str);
}

// TODO: split this function into smaller helpers
//      "clang-tidy: Function '_el_parse_primary' has cognitive complexity of 30 (threshold 25)" ~2026
ElAstExpr* _el_parse_primary(ElParser* parser) {
    if (_el_parser_is_type_literal(parser)) {
        ElStorageClass scls = EL_STORAGECLS_LOCAL;
        if (el_parser_match(parser, EL_TT_KW_STATIC)) {
            scls = EL_STORAGECLS_STATIC;
        }

        ElAstType* type = _el_parse_type(parser);
        ElAstInit* init = el_parse_init(parser);
        if (!init) return NULL;

        return el_ast_new_typedinit(parser->aarena, el_srcspan_merge(type->span, init->span), scls, type, init);
    }

    if (el_parser_check(parser, EL_TT_IDENT)) {
        ElToken tok = el_parser_advance(parser);
        return el_ast_new_ident(parser->aarena, tok.span, tok.lexeme);
    }

    if (el_parser_check(parser, EL_TT_INT_LITERAL)) {
        ElToken tok = el_parser_advance(parser);

        ElInt128 val = el_parse_int_lit(parser->diag, tok);
        return el_ast_new_int_lit(parser->aarena, tok.span, val);
    }

    if (el_parser_check(parser, EL_TT_FLOAT_LITERAL)) {
        ElToken tok = el_parser_advance(parser);

        double val = el_string_to_double(parser->diag, tok.lexeme, tok.span);
        return el_ast_new_float_lit(parser->aarena, tok.span, val);
    }

    // TODO: handle compile time concatenation, e.g. "foo" " bar" " baz"
    if (el_parser_check(parser, EL_TT_STRING_LITERAL)) {
        return parse_string_lit(parser);
    }

    if (el_parser_check(parser, EL_TT_CHAR_LITERAL)) {
        ElToken tok = el_parser_advance(parser);
        bool ok;

        char* buf = EL_DYNARENA_NEW_ARR(parser->farena, char, tok.lexeme.len);
        char c = el_parse_char_with_escapes(parser->diag, tok, buf, &ok);
        if (!ok) return NULL;

        return el_ast_new_char_lit(parser->aarena, tok.span, c);
    }

    if (el_parser_check(parser, EL_TT_TRUE_LITERAL)) {
        ElToken tok = el_parser_advance(parser);
        return el_ast_new_bool_lit(parser->aarena, tok.span, true);
    }

    if (el_parser_check(parser, EL_TT_FALSE_LITERAL)) {
        ElToken tok = el_parser_advance(parser);
        return el_ast_new_bool_lit(parser->aarena, tok.span, false);
    }

    if (el_parser_check(parser, EL_TT_NULL_LITERAL)) {
        ElToken tok = el_parser_advance(parser);
        return el_ast_new_null_lit(parser->aarena, tok.span);
    }

    if (el_parser_match(parser, EL_TT_LPAREN)) {
        ElAstExpr* expr = el_parse_expr(parser);
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

    if (parser->current.type != EL_TT_UNKNOWN || parser->diag->summary.total_errors == 0)
        _el_parser_report_unexpected(parser, parser->current);

    el_parser_advance(parser);
    return NULL;
}

ElAstExpr* _el_parse_member(ElParser* parser, ElAstExpr* expr, bool is_optional) {
    if (!expr) return NULL;
    if (el_parser_check(parser, EL_TT_IDENT)) {
        ElToken name_ident = el_parser_expect(parser, EL_TT_IDENT);
        ElSourceSpan span = el_srcspan_merge(expr->span, name_ident.span);
        return el_ast_new_member_expr(parser->aarena, span, expr, name_ident.lexeme, is_optional);
    } else if (el_parser_check(parser, EL_TT_INT_LITERAL)) {
        ElToken index_tok = el_parser_advance(parser);
        usize index;
        if (!_el_parse_const_idx(parser, index_tok, &index)) {
            return NULL;
        }

        ElSourceSpan span = el_srcspan_merge(expr->span, index_tok.span);
        return el_ast_new_tmember_expr(parser->aarena, span, expr, index, index_tok.span, is_optional);
    } else {
        el_parser_expect(parser, EL_TT_IDENT);
        return NULL;
    }
}

ElAstExpr* _el_parse_call(ElParser* parser, ElAstExpr* callee) {
    ElAstToI* args_head = NULL;
    ElAstToI* args_tail = NULL;
    usize     arg_count = 0;

    if (!el_parser_check(parser, EL_TT_RPAREN)) {
        while (true) {
            ElAstToI* arg = el_parse_toi(parser);
            if (el_parser_has_errs(parser)) {
                el_parser_sync(parser, EL_PARSER_SYNC_EXPR);
                if (!el_parser_check(parser, EL_TT_COMMA)) {
                    break;
                }
                continue;
            }

            el_ast_append_toi(&args_head, &args_tail, arg);
            arg_count++;

            if (!el_parser_match(parser, EL_TT_COMMA)) break;
            if (el_parser_check(parser, EL_TT_RPAREN)) break;
        }
    }

    ElToken rparen = parser->current;
    if (el_parser_check(parser, EL_TT_RPAREN)) {
        rparen = el_parser_advance(parser);
    } else {
        el_parser_expect(parser, EL_TT_RPAREN);
        el_parser_skip_to(parser, EL_TT_RPAREN);
        if (el_parser_check(parser, EL_TT_RPAREN)) {
            rparen = el_parser_advance(parser);
        }
    }

    return el_ast_new_call_expr(
        parser->aarena,
        el_srcspan_merge(callee->span, rparen.span),
        callee, args_head, arg_count
    );
}

ElAstExpr* _el_parse_postfix(ElParser* parser) {
    ElAstExpr* expr = _el_parse_primary(parser);
    if (el_parser_has_errs(parser)) return NULL;

    while (true) {
        if (el_parser_check(parser, EL_TT_INC)) {
            ElToken tok = el_parser_advance(parser);
            expr = el_ast_new_unary_expr(parser->aarena, el_srcspan_merge(expr->span, tok.span), EL_UNARY_OP_POST_INC, expr);
        } else if (el_parser_check(parser, EL_TT_DEC)) {
            ElToken tok = el_parser_advance(parser);
            expr = el_ast_new_unary_expr(parser->aarena, el_srcspan_merge(expr->span, tok.span), EL_UNARY_OP_POST_DEC, expr);
        } else if (el_parser_match(parser, EL_TT_LPAREN)) {
            expr = _el_parse_call(parser, expr);
        } else if (el_parser_check(parser, EL_TT_CARET)) {
            ElToken tok = el_parser_advance(parser);
            expr = el_ast_new_unary_expr(parser->aarena, el_srcspan_merge(expr->span, tok.span), EL_UNARY_OP_DEREF, expr);
        } else if (el_parser_check(parser, EL_TT_LOGICAL_NOT)) {
            ElToken tok = el_parser_advance(parser);
            expr = el_ast_new_unary_expr(parser->aarena, el_srcspan_merge(expr->span, tok.span), EL_UNARY_OP_OPT_UNWRAP, expr);
        } else if (el_parser_match(parser, EL_TT_LBRACKET)) {
            ElAstExpr* index = el_parse_expr(parser);
            if (el_parser_has_errs(parser)) {
                el_parser_sync(parser, EL_PARSER_SYNC_EXPR);
            }

            ElToken rbracket = parser->current;
            if (el_parser_check(parser, EL_TT_RBRACKET)) {
                rbracket = el_parser_advance(parser);
            } else {
                el_parser_expect(parser, EL_TT_RBRACKET);
                el_parser_skip_to(parser, EL_TT_RBRACKET);
                if (el_parser_check(parser, EL_TT_RBRACKET)) {
                    rbracket = el_parser_advance(parser);
                }
            }

            expr = el_ast_new_bin_expr(
                parser->aarena,
                el_srcspan_merge(expr->span, rbracket.span),
                EL_BIN_OP_INDEX, expr, index
            );
        } else if (el_parser_match(parser, EL_TT_DOT)) {
            expr = _el_parse_member(parser, expr, false);
        } else if (el_parser_match(parser, EL_TT_OPT_DOT)) {
            expr = _el_parse_member(parser, expr, true);
        } else {
            break;
        }

        if (el_parser_has_errs(parser)) return NULL;
    }
    return expr;
}

ElAstExpr* _el_parse_unary(ElParser* parser) {
    if (el_parser_check(parser, EL_TT_PLUS)) {
        ElToken tok = el_parser_advance(parser);
        ElAstExpr* operand = _el_parse_unary(parser);
        if (el_parser_has_errs(parser)) return NULL;
        return el_ast_new_unary_expr(parser->aarena, el_srcspan_merge(tok.span, operand->span), EL_UNARY_OP_POS, operand);
    }
    if (el_parser_check(parser, EL_TT_MINUS)) {
        ElToken tok = el_parser_advance(parser);
        ElAstExpr* operand = _el_parse_unary(parser);
        if (el_parser_has_errs(parser)) return NULL;
        return el_ast_new_unary_expr(parser->aarena, el_srcspan_merge(tok.span, operand->span), EL_UNARY_OP_NEG, operand);
    }
    if (el_parser_check(parser, EL_TT_LOGICAL_NOT)) {
        ElToken tok = el_parser_advance(parser);
        ElAstExpr* operand = _el_parse_unary(parser);
        if (el_parser_has_errs(parser)) return NULL;
        return el_ast_new_unary_expr(parser->aarena, el_srcspan_merge(tok.span, operand->span), EL_UNARY_OP_NOT, operand);
    }
    if (el_parser_check(parser, EL_TT_BITWISE_NOT)) {
        ElToken tok = el_parser_advance(parser);
        ElAstExpr* operand = _el_parse_unary(parser);
        if (el_parser_has_errs(parser)) return NULL;
        return el_ast_new_unary_expr(parser->aarena, el_srcspan_merge(tok.span, operand->span), EL_UNARY_OP_BW_NOT, operand);
    }
    if (el_parser_check(parser, EL_TT_INC)) {
        ElToken tok = el_parser_advance(parser);
        ElAstExpr* operand = _el_parse_unary(parser);
        if (el_parser_has_errs(parser)) return NULL;
        return el_ast_new_unary_expr(parser->aarena, el_srcspan_merge(tok.span, operand->span), EL_UNARY_OP_PRE_INC, operand);
    }
    if (el_parser_check(parser, EL_TT_DEC)) {
        ElToken tok = el_parser_advance(parser);
        ElAstExpr* operand = _el_parse_unary(parser);
        if (el_parser_has_errs(parser)) return NULL;
        return el_ast_new_unary_expr(parser->aarena, el_srcspan_merge(tok.span, operand->span), EL_UNARY_OP_PRE_DEC, operand);
    }
    if (el_parser_check(parser, EL_TT_BITWISE_AND)) {
        ElToken tok = el_parser_advance(parser);
        ElAstExpr* operand = _el_parse_unary(parser);
        if (el_parser_has_errs(parser)) return NULL;
        return el_ast_new_unary_expr(parser->aarena, el_srcspan_merge(tok.span, operand->span), EL_UNARY_OP_ADDROF, operand);
    }

    return _el_parse_postfix(parser);
}

ElAstExpr* _el_parse_cast(ElParser* parser) {
    ElAstExpr* expr = _el_parse_unary(parser);
    if (el_parser_has_errs(parser)) return NULL;

    while (true) {
        ElCastKind kind;
        if (el_parser_match(parser, EL_TT_KW_AS)) {
            kind = EL_SEMCAST;
        } else if (el_parser_match(parser, EL_TT_KW_BITCAST)) {
            kind = EL_BITCAST;
        } else {
            break;
        }

        ElAstType* type = _el_parse_type(parser);
        if (type == NULL) return NULL;
        expr = el_ast_new_cast_expr(parser->aarena, el_srcspan_merge(expr->span, type->span), kind, expr, type);
    }
    return expr;
}

ElAstExpr* _el_parse_multiplicative(ElParser* parser) {
    ElAstExpr* expr = _el_parse_cast(parser);
    if (el_parser_has_errs(parser)) return NULL;

    while (true) {
        ElAstBinOp type;
        if (el_parser_match(parser, EL_TT_STAR)) type = EL_BIN_OP_MUL;
        else if (el_parser_match(parser, EL_TT_SLASH)) type = EL_BIN_OP_DIV;
        else if (el_parser_match(parser, EL_TT_PERCENT)) type = EL_BIN_OP_MOD;
        else break;

        ElAstExpr* right = _el_parse_cast(parser);
        if (el_parser_has_errs(parser)) {
            el_parser_sync(parser, EL_PARSER_SYNC_EXPR);
            break;
        }
        expr = el_ast_new_bin_expr(parser->aarena, el_srcspan_merge(expr->span, right->span), type, expr, right);
    }
    return expr;
}

ElAstExpr* _el_parse_additive(ElParser* parser) {
    ElAstExpr* expr = _el_parse_multiplicative(parser);
    if (el_parser_has_errs(parser)) return NULL;

    while (true) {
        ElAstBinOp type;
        if (el_parser_match(parser, EL_TT_PLUS)) type = EL_BIN_OP_ADD;
        else if (el_parser_match(parser, EL_TT_MINUS)) type = EL_BIN_OP_SUB;
        else break;

        ElAstExpr* right = _el_parse_multiplicative(parser);
        if (el_parser_has_errs(parser)) {
            el_parser_sync(parser, EL_PARSER_SYNC_EXPR);
            break;
        }
        expr = el_ast_new_bin_expr(parser->aarena, el_srcspan_merge(expr->span, right->span), type, expr, right);
    }
    return expr;
}

ElAstExpr* _el_parse_shift(ElParser* parser) {
    ElAstExpr* expr = _el_parse_additive(parser);
    if (el_parser_has_errs(parser)) return NULL;

    while (true) {
        ElAstBinOp type;
        if (el_parser_match(parser, EL_TT_SHL)) type = EL_BIN_OP_SHL;
        else if (el_parser_match(parser, EL_TT_SHR)) type = EL_BIN_OP_SHR;
        else break;

        ElAstExpr* right = _el_parse_additive(parser);
        if (el_parser_has_errs(parser)) {
            el_parser_sync(parser, EL_PARSER_SYNC_EXPR);
            break;
        }
        expr = el_ast_new_bin_expr(parser->aarena, el_srcspan_merge(expr->span, right->span), type, expr, right);
    }
    return expr;
}

ElAstExpr* _el_parse_relational(ElParser* parser) {
    ElAstExpr* expr = _el_parse_shift(parser);
    if (el_parser_has_errs(parser)) return NULL;

    while (true) {
        ElAstBinOp type;
        if (el_parser_match(parser, EL_TT_LT)) type = EL_BIN_OP_LT;
        else if (el_parser_match(parser, EL_TT_LTE)) type = EL_BIN_OP_LTE;
        else if (el_parser_match(parser, EL_TT_GT)) type = EL_BIN_OP_GT;
        else if (el_parser_match(parser, EL_TT_GTE)) type = EL_BIN_OP_GTE;
        else break;

        ElAstExpr* right = _el_parse_shift(parser);
        if (el_parser_has_errs(parser)) {
            el_parser_sync(parser, EL_PARSER_SYNC_EXPR);
            break;
        }
        expr = el_ast_new_bin_expr(parser->aarena, el_srcspan_merge(expr->span, right->span), type, expr, right);
    }
    return expr;
}

ElAstExpr* _el_parse_equality(ElParser* parser) {
    ElAstExpr* expr = _el_parse_relational(parser);
    if (el_parser_has_errs(parser)) return NULL;

    while (true) {
        ElAstBinOp type;
        if (el_parser_match(parser, EL_TT_EQL)) type = EL_BIN_OP_EQ;
        else if (el_parser_match(parser, EL_TT_NEQ)) type = EL_BIN_OP_NEQ;
        else break;

        ElAstExpr* right = _el_parse_relational(parser);
        if (el_parser_has_errs(parser)) {
            el_parser_sync(parser, EL_PARSER_SYNC_EXPR);
            break;
        }
        expr = el_ast_new_bin_expr(parser->aarena, el_srcspan_merge(expr->span, right->span), type, expr, right);
    }
    return expr;
}

ElAstExpr* _el_parse_bitwise_and(ElParser* parser) {
    ElAstExpr* expr = _el_parse_equality(parser);
    if (el_parser_has_errs(parser)) return NULL;

    while (el_parser_match(parser, EL_TT_BITWISE_AND)) {
        ElAstExpr* right = _el_parse_equality(parser);
        if (el_parser_has_errs(parser)) {
            el_parser_sync(parser, EL_PARSER_SYNC_EXPR);
            break;
        }
        expr = el_ast_new_bin_expr(parser->aarena, el_srcspan_merge(expr->span, right->span), EL_BIN_OP_BW_AND, expr, right);
    }
    return expr;
}

ElAstExpr* _el_parse_bitwise_xor(ElParser* parser) {
    ElAstExpr* expr = _el_parse_bitwise_and(parser);
    if (el_parser_has_errs(parser)) return NULL;

    while (el_parser_match(parser, EL_TT_BITWISE_XOR)) {
        ElAstExpr* right = _el_parse_bitwise_and(parser);
        if (el_parser_has_errs(parser)) {
            el_parser_sync(parser, EL_PARSER_SYNC_EXPR);
            break;
        }
        expr = el_ast_new_bin_expr(parser->aarena, el_srcspan_merge(expr->span, right->span), EL_BIN_OP_BW_XOR, expr, right);
    }
    return expr;
}

ElAstExpr* _el_parse_bitwise_or(ElParser* parser) {
    ElAstExpr* expr = _el_parse_bitwise_xor(parser);
    if (el_parser_has_errs(parser)) return NULL;

    while (el_parser_match(parser, EL_TT_BITWISE_OR)) {
        ElAstExpr* right = _el_parse_bitwise_xor(parser);
        if (el_parser_has_errs(parser)) {
            el_parser_sync(parser, EL_PARSER_SYNC_EXPR);
            break;
        }
        expr = el_ast_new_bin_expr(parser->aarena, el_srcspan_merge(expr->span, right->span), EL_BIN_OP_BW_OR, expr, right);
    }
    return expr;
}

ElAstExpr* _el_parse_bitwise_imp(ElParser* parser) {
    ElAstExpr* expr = _el_parse_bitwise_or(parser);
    if (el_parser_has_errs(parser)) return NULL;

    while (el_parser_match(parser, EL_TT_BITWISE_IMP)) {
        ElAstExpr* right = _el_parse_bitwise_or(parser);
        if (el_parser_has_errs(parser)) {
            el_parser_sync(parser, EL_PARSER_SYNC_EXPR);
            break;
        }
        expr = el_ast_new_bin_expr(parser->aarena, el_srcspan_merge(expr->span, right->span), EL_BIN_OP_BW_IMP, expr, right);
    }
    return expr;
}

ElAstExpr* _el_parse_logical_and(ElParser* parser) {
    ElAstExpr* expr = _el_parse_bitwise_imp(parser);
    if (el_parser_has_errs(parser)) return NULL;

    while (el_parser_match(parser, EL_TT_LOGICAL_AND)) {
        ElAstExpr* right = _el_parse_bitwise_imp(parser);
        if (el_parser_has_errs(parser)) {
            el_parser_sync(parser, EL_PARSER_SYNC_EXPR);
            break;
        }
        expr = el_ast_new_bin_expr(parser->aarena, el_srcspan_merge(expr->span, right->span), EL_BIN_OP_AND, expr, right);
    }
    return expr;
}

ElAstExpr* _el_parse_logical_or(ElParser* parser) {
    ElAstExpr* expr = _el_parse_logical_and(parser);
    if (el_parser_has_errs(parser)) return NULL;

    while (el_parser_match(parser, EL_TT_LOGICAL_OR)) {
        ElAstExpr* right = _el_parse_logical_and(parser);
        if (el_parser_has_errs(parser)) {
            el_parser_sync(parser, EL_PARSER_SYNC_EXPR);
            break;
        }
        expr = el_ast_new_bin_expr(parser->aarena, el_srcspan_merge(expr->span, right->span), EL_BIN_OP_OR, expr, right);
    }
    return expr;
}

ElAstExpr* _el_parse_logical_imp(ElParser* parser) {
    ElAstExpr* expr = _el_parse_logical_or(parser);
    if (el_parser_has_errs(parser)) return NULL;

    while (el_parser_match(parser, EL_TT_LOGICAL_IMP)) {
        ElAstExpr* right = _el_parse_logical_or(parser);
        if (el_parser_has_errs(parser)) {
            el_parser_sync(parser, EL_PARSER_SYNC_EXPR);
            break;
        }
        expr = el_ast_new_bin_expr(parser->aarena, el_srcspan_merge(expr->span, right->span), EL_BIN_OP_IMP, expr, right);
    }
    return expr;
}

ElAstExpr* _el_parse_optional(ElParser* parser) {
    ElAstExpr* expr = _el_parse_logical_imp(parser);
    if (el_parser_has_errs(parser)) return NULL;

    while (true) {
        ElAstBinOp type;
        if (el_parser_match(parser, EL_TT_OPT_FB)) type = EL_BIN_OP_OPT_FB;
        else if (el_parser_match(parser, EL_TT_OPT_MAP)) type = EL_BIN_OP_OPT_MAP;
        else break;

        ElAstExpr* right = _el_parse_logical_imp(parser);
        if (el_parser_has_errs(parser)) {
            el_parser_sync(parser, EL_PARSER_SYNC_EXPR);
            break;
        }
        expr = el_ast_new_bin_expr(parser->aarena, el_srcspan_merge(expr->span, right->span), type, expr, right);
    }
    return expr;
}

ElAstExpr* el_parse_expr(ElParser* parser) {
    el_prof_begin_sub(parser->prof, parser->pss_expr);
    ElAstExpr* result = _el_parse_optional(parser);
    el_prof_finish_sub(parser->prof, parser->pss_expr);
    return result;
}
