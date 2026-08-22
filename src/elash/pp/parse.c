#include "preproc-internals.h"

#include <elash/diag/engine.h>

#include <elash/lexer/token.h>
#include <elash/source/span.h>

#include <elash/sema/bin-op.h>
#include <elash/sema/unary-op.h>
#include <elash/pp/valbuf.h>
#include <elash/pp/value.h>

#include <elash/util/dynarena.h>
#include <elash/util/strconv.h>
#include <elash/util/assert.h>

static bool peek(ElPreproc* pp, ElToken* out_tok) {
    return _el_pp_peek(pp, out_tok);
}

// TODO: move this function to some shared module and implement
//       something similar for parsing string literals; then reuse
//       them in the parser
static char parse_char_literal(ElStringView lexeme) {
    if (lexeme.len == 0) {
        return '\0';
    }
    if (lexeme.data[0] != '\\') {
        return lexeme.data[0];
    }
    if (lexeme.len < 2) {
        return lexeme.data[0];
    }
    switch (lexeme.data[1]) {
    case 'n':  return '\n';
    case 't':  return '\t';
    case 'r':  return '\r';
    case '0':  return '\0';
    case '\\': return '\\';
    case '\'': return '\'';
    case '"':  return '"';
    default:   return lexeme.data[1];
    }
}

static ElPpValue* parse_list_literal(ElPreproc* pp, ElSourceSpan open_span) {
    ElPpValBuf buf;
    el_pp_valbuf_init(&buf);

    if (!_el_pp_match(pp, EL_TT_RBRACE)) {
        while (true) {
            ElPpValue* elem = _el_pp_eval(pp);
            if (elem == NULL) {
                el_pp_valbuf_free(&buf);
                return NULL;
            }

            el_pp_valbuf_push(&buf, elem);

            if (_el_pp_match(pp, EL_TT_RBRACE)) {
                break;
            }
            if (!_el_pp_match(pp, EL_TT_COMMA)) {
                el_pp_valbuf_free(&buf);
                return el_diag_report(
                    pp->diag, EL_DIAG_ERROR, "pp.unexpected-token", open_span,
                    "expected ',' or '}' in list literal"
                );
            }

            if (_el_pp_match(pp, EL_TT_RBRACE)) {
                break;
            }
        }
    }

    return el_pp_valbuf_vflush(&buf, pp->iarena);
}

static ElPpValue* parse_token_quote(ElPreproc* pp, ElSourceSpan hash_span) {
    _el_pp_advance(pp); // <
    ElToken inner = _el_pp_advance(pp);
    if (inner.type == EL_TT_EOF) {
        return el_diag_report(
            pp->diag, EL_DIAG_ERROR, "pp.unexpected-token", hash_span,
            "unexpected end of file after the single token quote expression",
        );
    }

    ElToken close = _el_pp_advance(pp);
    if (close.type != EL_TT_GT) {
        el_diag_report(
            pp->diag, EL_DIAG_ERROR, "pp.invalid-quote", hash_span,
            "token quote '#<...>' must contain exactly one token",
        );
        el_diag_help(
            pp->diag, "found ${tok} before '>'",
            EL_DIAG_STRING("tok", el_token_type_format(close.type)),
        );
        return NULL;
    }

    return _el_pp_new_tok(pp->iarena, inner);
}

static ElPpValue* parse_token_list(ElPreproc* pp, ElSourceSpan hash_span) {
    _el_pp_advance(pp); // (
    ElPpValBuf buf;
    el_pp_valbuf_init(&buf);

    uint depth = 1;
    while (depth > 0) {
        ElToken tok = _el_pp_advance(pp);
        if (tok.type == EL_TT_EOF) {
            el_pp_valbuf_free(&buf);
            return _el_pp_report_unterm_quote(pp, hash_span);
        }

        if (tok.type == EL_TT_LPAREN) {
            depth++;
        } else if (tok.type == EL_TT_RPAREN) {
            depth--;
            if (depth == 0)
                break;
        }

        ElPpValue* elem = _el_pp_new_tok(pp->iarena, tok);
        el_pp_valbuf_push(&buf, elem);
    }

    return el_pp_valbuf_vflush(&buf, pp->iarena);
}

static ElPpValue* parse_primary(ElPreproc* pp) {
    ElToken tok;
    if (!peek(pp, &tok)) {
        return el_diag_report(
            pp->diag, EL_DIAG_ERROR, "pp.unexpected-token",
            EL_SRCSPAN_NULL, "unexpected end of input in preprocessor expression"
        );
    }

    if (tok.type == EL_TT_HASH) {
        ElSourceSpan hash_span = tok.span;
        _el_pp_advance(pp);

        ElToken next;
        if (!peek(pp, &next)) {
            return el_diag_report(
                pp->diag, EL_DIAG_ERROR, "pp.invalid-quote", hash_span,
                "expected '<' or '(' after '#'"
            );
        }

        if (next.type == EL_TT_LT)
            return parse_token_quote(pp, hash_span);
        if (next.type == EL_TT_LPAREN)
            return parse_token_list(pp, hash_span);

        return el_diag_report(
            pp->diag, EL_DIAG_ERROR, "pp.invalid-quote", hash_span,
            "expected '<' or '(' after '#'"
        );
    }

    if (tok.type == EL_TT_LBRACE) {
        ElSourceSpan open_span = tok.span;
        _el_pp_advance(pp);
        return parse_list_literal(pp, open_span);
    }

    _el_pp_advance(pp);

    switch (tok.type) {
    case EL_TT_INT_LITERAL: {
        int64_t val = 0;
        if (!el_string_to_i64(tok.lexeme, /* NOLINTBEGIN(readability-magic-numbers): Keep Yourself Safe */ 10 /* NOLINTEND(readability-magic-numbers) */, &val)) {
            return el_diag_report(
                pp->diag, EL_DIAG_ERROR, "pp.invalid-literal",
                tok.span, "invalid integer literal"
            );
        }
        return _el_pp_new_int(pp->iarena, val);
    }
    case EL_TT_FLOAT_LITERAL: {
        long double val = 0.0L;
        if (!el_string_to_long_double(tok.lexeme, &val)) {
            return el_diag_report(
                pp->diag, EL_DIAG_ERROR, "pp.invalid-literal",
                tok.span, "invalid float literal"
            );
        }
        return _el_pp_new_float(pp->iarena, (double)val);
    }
    case EL_TT_CHAR_LITERAL:
        return _el_pp_new_char(pp->iarena, parse_char_literal(tok.lexeme));
    case EL_TT_STRING_LITERAL:
        return _el_pp_new_str(pp->iarena, tok.lexeme);
    case EL_TT_TRUE_LITERAL:
        return _el_pp_new_bool(pp->iarena, true);
    case EL_TT_FALSE_LITERAL:
        return _el_pp_new_bool(pp->iarena, false);
    case EL_TT_NULL_LITERAL:
        return _el_pp_new_null(pp->iarena);
    case EL_TT_LPAREN: {
        ElPpValue* expr = _el_pp_eval(pp);
        if (expr == NULL) {
            return NULL;
        }
        if (!_el_pp_expect(pp, EL_TT_RPAREN)) {
            return NULL;
        }
        return expr;
    }

    case EL_TT_IDENT: {
        ElPpSymbol* sym = el_pp_scope_lookup(pp->current_scope, tok.lexeme);
        if (sym == NULL) {
            return el_diag_report(
                pp->diag, EL_DIAG_ERROR, "pp.undeclared", tok.span,
                "undeclared identifier '${name}' in preprocessor expression",
                EL_DIAG_STRING("name", tok.lexeme)
            );
        }
        if (sym->kind != EL_PP_SYM_VAR) {
            return el_diag_report(
                pp->diag, EL_DIAG_ERROR, "pp.sym-kind",
                tok.span, "expected a variable name"
            );
        }

        return _el_pp_value_clone(pp->iarena, sym->as.var.v);
    }

    default:
        return el_diag_report(
            pp->diag, EL_DIAG_ERROR, "pp.unexpected-token", tok.span,
            "unexpected token ${tok} in preprocessor expression",
            EL_DIAG_STRING("tok", el_token_type_format(tok.type))
        );
    }
}

static ElPpValue* parse_postfix(ElPreproc* pp) {
    ElPpValue* expr = parse_primary(pp);
    if (expr == NULL) {
        return NULL;
    }

    while (true) {
        ElToken tok;
        if (!peek(pp, &tok)) {
            break;
        }

        if (tok.type == EL_TT_INC || tok.type == EL_TT_DEC) {
            return _el_pp_report_incdec(pp, tok.span);
        } else if (tok.type == EL_TT_CARET) {
            return _el_pp_report_deref(pp, tok.span);
        } else if (tok.type == EL_TT_LBRACKET) {
            ElSourceSpan open_span = tok.span;
            _el_pp_advance(pp);
            ElPpValue* index = _el_pp_eval(pp);
            if (index == NULL) {
                return NULL;
            }
            if (!_el_pp_expect(pp, EL_TT_RBRACKET)) {
                return NULL;
            }
            expr = _el_pp_apply_bin_op(
                pp, el_srcspan_merge(open_span, tok.span),
                EL_SEMA_BIN_OP_INDEX, expr, index
            );
        } else {
            break;
        }

        if (expr == NULL) {
            return NULL;
        }
    }

    return expr;
}

static ElPpValue* parse_unary(ElPreproc* pp) {
    ElToken tok;
    if (!peek(pp, &tok)) {
        return parse_postfix(pp);
    }

    ElSemaUnaryOp op;
    switch (tok.type) {
    case EL_TT_PLUS:        op = EL_SEMA_UNARY_OP_POS;    break;
    case EL_TT_MINUS:       op = EL_SEMA_UNARY_OP_NEG;    break;
    case EL_TT_LOGICAL_NOT: op = EL_SEMA_UNARY_OP_NOT;    break;
    case EL_TT_BITWISE_NOT: op = EL_SEMA_UNARY_OP_BW_NOT; break;
    default:                return parse_postfix(pp);
    }

    _el_pp_advance(pp);
    ElPpValue* operand = parse_unary(pp);
    if (operand == NULL) return NULL;
    return _el_pp_apply_unary_op(pp, tok.span, op, operand);
}

static ElPpValue* parse_multiplicative(ElPreproc* pp) {
    ElPpValue* expr = parse_unary(pp);
    if (expr == NULL) return NULL;

    while (true) {
        ElSemaBinOp op;
        ElToken tok;
        if (!peek(pp, &tok)) break;

        if (tok.type == EL_TT_STAR) op = EL_SEMA_BIN_OP_MUL;
        else if (tok.type == EL_TT_SLASH) op = EL_SEMA_BIN_OP_DIV;
        else if (tok.type == EL_TT_PERCENT) op = EL_SEMA_BIN_OP_MOD;
        else break;

        _el_pp_advance(pp);
        ElPpValue* rhs = parse_unary(pp);
        if (rhs == NULL) return NULL;
        expr = _el_pp_apply_bin_op(pp, tok.span, op, expr, rhs);
        if (expr == NULL) return NULL;
    }

    return expr;
}

static ElPpValue* parse_additive(ElPreproc* pp) {
    ElPpValue* expr = parse_multiplicative(pp);
    if (expr == NULL) return NULL;

    while (true) {
        ElSemaBinOp op;
        ElToken tok;
        if (!peek(pp, &tok)) break;

        if (tok.type == EL_TT_PLUS) op = EL_SEMA_BIN_OP_ADD;
        else if (tok.type == EL_TT_MINUS) op = EL_SEMA_BIN_OP_SUB;
        else break;

        _el_pp_advance(pp);
        ElPpValue* rhs = parse_multiplicative(pp);
        if (rhs == NULL) return NULL;
        expr = _el_pp_apply_bin_op(pp, tok.span, op, expr, rhs);
        if (expr == NULL) return NULL;
    }

    return expr;
}

static ElPpValue* parse_shift(ElPreproc* pp) {
    ElPpValue* expr = parse_additive(pp);
    if (expr == NULL) return NULL;

    while (true) {
        ElSemaBinOp op;
        ElToken tok;
        if (!peek(pp, &tok)) break;

        if (tok.type == EL_TT_SHL) op = EL_SEMA_BIN_OP_SHL;
        else if (tok.type == EL_TT_SHR) op = EL_SEMA_BIN_OP_SHR;
        else break;

        _el_pp_advance(pp);
        ElPpValue* rhs = parse_additive(pp);
        if (rhs == NULL) return NULL;
        expr = _el_pp_apply_bin_op(pp, tok.span, op, expr, rhs);
        if (expr == NULL) return NULL;
    }

    return expr;
}

static ElPpValue* parse_relational(ElPreproc* pp) {
    ElPpValue* expr = parse_shift(pp);
    if (expr == NULL) return NULL;

    while (true) {
        ElSemaBinOp op;
        ElToken tok;
        if (!peek(pp, &tok)) break;

        if (tok.type == EL_TT_LT) op = EL_SEMA_BIN_OP_LT;
        else if (tok.type == EL_TT_LTE) op = EL_SEMA_BIN_OP_LTE;
        else if (tok.type == EL_TT_GT) op = EL_SEMA_BIN_OP_GT;
        else if (tok.type == EL_TT_GTE) op = EL_SEMA_BIN_OP_GTE;
        else break;

        _el_pp_advance(pp);
        ElPpValue* rhs = parse_shift(pp);
        if (rhs == NULL) return NULL;
        expr = _el_pp_apply_bin_op(pp, tok.span, op, expr, rhs);
        if (expr == NULL) return NULL;
    }

    return expr;
}

static ElPpValue* parse_equality(ElPreproc* pp) {
    ElPpValue* expr = parse_relational(pp);
    if (expr == NULL) return NULL;

    while (true) {
        ElSemaBinOp op;
        ElToken tok;
        if (!peek(pp, &tok)) break;

        if (tok.type == EL_TT_EQL) op = EL_SEMA_BIN_OP_EQ;
        else if (tok.type == EL_TT_NEQ) op = EL_SEMA_BIN_OP_NEQ;
        else break;

        _el_pp_advance(pp);
        ElPpValue* rhs = parse_relational(pp);
        if (rhs == NULL) return NULL;
        expr = _el_pp_apply_bin_op(pp, tok.span, op, expr, rhs);
        if (expr == NULL) return NULL;
    }

    return expr;
}

static ElPpValue* parse_bitwise_and(ElPreproc* pp) {
    ElPpValue* expr = parse_equality(pp);
    if (expr == NULL) return NULL;

    while (true) {
        ElToken tok;
        if (!peek(pp, &tok) || tok.type != EL_TT_BITWISE_AND) break;
        _el_pp_advance(pp);
        ElPpValue* rhs = parse_equality(pp);
        if (rhs == NULL) return NULL;
        expr = _el_pp_apply_bin_op(pp, tok.span, EL_SEMA_BIN_OP_BW_AND, expr, rhs);
        if (expr == NULL) return NULL;
    }

    return expr;
}

static ElPpValue* parse_bitwise_xor(ElPreproc* pp) {
    ElPpValue* expr = parse_bitwise_and(pp);
    if (expr == NULL) return NULL;

    while (true) {
        ElToken tok;
        if (!peek(pp, &tok) || tok.type != EL_TT_BITWISE_XOR) break;
        _el_pp_advance(pp);
        ElPpValue* rhs = parse_bitwise_and(pp);
        if (rhs == NULL) return NULL;
        expr = _el_pp_apply_bin_op(pp, tok.span, EL_SEMA_BIN_OP_BW_XOR, expr, rhs);
        if (expr == NULL) return NULL;
    }

    return expr;
}

static ElPpValue* parse_bitwise_or(ElPreproc* pp) {
    ElPpValue* expr = parse_bitwise_xor(pp);
    if (expr == NULL) return NULL;

    while (true) {
        ElToken tok;
        if (!peek(pp, &tok) || tok.type != EL_TT_BITWISE_OR) break;
        _el_pp_advance(pp);
        ElPpValue* rhs = parse_bitwise_xor(pp);
        if (rhs == NULL) return NULL;
        expr = _el_pp_apply_bin_op(pp, tok.span, EL_SEMA_BIN_OP_BW_OR, expr, rhs);
        if (expr == NULL) return NULL;
    }

    return expr;
}

static ElPpValue* parse_bitwise_imp(ElPreproc* pp) {
    ElPpValue* expr = parse_bitwise_or(pp);
    if (expr == NULL) return NULL;

    while (true) {
        ElToken tok;
        if (!peek(pp, &tok) || tok.type != EL_TT_BITWISE_IMP) break;
        _el_pp_advance(pp);
        ElPpValue* rhs = parse_bitwise_or(pp);
        if (rhs == NULL) return NULL;
        expr = _el_pp_apply_bin_op(pp, tok.span, EL_SEMA_BIN_OP_BW_IMP, expr, rhs);
        if (expr == NULL) return NULL;
    }

    return expr;
}

static ElPpValue* parse_logical_and(ElPreproc* pp) {
    ElPpValue* expr = parse_bitwise_imp(pp);
    if (expr == NULL) return NULL;

    while (true) {
        ElToken tok;
        if (!peek(pp, &tok) || tok.type != EL_TT_LOGICAL_AND) break;
        _el_pp_advance(pp);
        ElPpValue* rhs = parse_bitwise_imp(pp);
        if (rhs == NULL) return NULL;
        expr = _el_pp_apply_bin_op(pp, tok.span, EL_SEMA_BIN_OP_AND, expr, rhs);
        if (expr == NULL) return NULL;
    }

    return expr;
}

static ElPpValue* parse_logical_or(ElPreproc* pp) {
    ElPpValue* expr = parse_logical_and(pp);
    if (expr == NULL) return NULL;

    while (true) {
        ElToken tok;
        if (!peek(pp, &tok) || tok.type != EL_TT_LOGICAL_OR) break;
        _el_pp_advance(pp);
        ElPpValue* rhs = parse_logical_and(pp);
        if (rhs == NULL) return NULL;
        expr = _el_pp_apply_bin_op(pp, tok.span, EL_SEMA_BIN_OP_OR, expr, rhs);
        if (expr == NULL) return NULL;
    }

    return expr;
}

ElPpValue* _el_pp_eval(ElPreproc* pp) {
    ElPpValue* expr = parse_logical_or(pp);
    if (expr == NULL) return NULL;

    while (true) {
        ElToken tok;
        if (!peek(pp, &tok) || tok.type != EL_TT_LOGICAL_IMP) break;
        _el_pp_advance(pp);
        ElPpValue* rhs = parse_logical_or(pp);
        if (rhs == NULL) return NULL;
        expr = _el_pp_apply_bin_op(pp, tok.span, EL_SEMA_BIN_OP_IMP, expr, rhs);
        if (expr == NULL) return NULL;
    }

    return expr;
}
