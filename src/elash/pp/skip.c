#include "preproc-internals.h"

// NOLINTNEXTLINE(readability-function-cognitive-complexity): let the world burn
static bool skip_primary(ElPreproc* pp) {
    ElToken tok;
    if (!_el_pp_read(pp, &tok)) return false;

    if (tok.type == EL_TT_HASH) {
        ElToken next;
        if (!_el_pp_read(pp, &next)) return false;
        if (next.type == EL_TT_LT) {
            // i'm unsure if we should validate that there is exactly one token
            // maybe not. it;s a skip function.
            while (_el_pp_read(pp, &tok) && tok.type != EL_TT_GT);
        } else if (next.type == EL_TT_LPAREN) {
            uint depth = 1;
            while (depth > 0 && _el_pp_read(pp, &tok)) {
                if (tok.type == EL_TT_LPAREN) depth++;
                else if (tok.type == EL_TT_RPAREN) depth--;
            }

            if (depth != 0) return _el_pp_report_unterm_quote(pp, tok.span);
        }
        return true;
    }

    if (tok.type == EL_TT_LBRACE) {
        uint depth = 1;
        while (depth > 0 && _el_pp_read(pp, &tok)) {
            if (tok.type == EL_TT_LBRACE) depth++;
            else if (tok.type == EL_TT_RBRACE) depth--;
        }

        if (depth != 0)
            return _el_pp_expect(pp, EL_TT_RBRACE);
    }

    if (tok.type == EL_TT_LPAREN) {
        return _el_pp_skip_expr(pp) && _el_pp_match(pp, EL_TT_RPAREN);
    }

    return true;
}

static bool skip_postfix(ElPreproc* pp) {
    if (!skip_primary(pp)) return false;

    while (true) {
        ElToken tok;
        if (!_el_pp_peek(pp, &tok)) break;

        if (tok.type == EL_TT_INC || tok.type == EL_TT_DEC || tok.type == EL_TT_CARET) {
            _el_pp_advance(pp);
        } else if (tok.type == EL_TT_LBRACKET) {
            _el_pp_advance(pp);
            if (!_el_pp_skip_expr(pp)) return false;
            if (!_el_pp_match(pp, EL_TT_RBRACKET)) return false;
        } else {
            break;
        }
    }
    return true;
}

static bool skip_unary(ElPreproc* pp) {
    ElToken tok;
    if (!_el_pp_peek(pp, &tok)) return skip_postfix(pp);

    switch (tok.type) {
    case EL_TT_PLUS:
    case EL_TT_MINUS:
    case EL_TT_LOGICAL_NOT:
    case EL_TT_BITWISE_NOT:
        _el_pp_advance(pp);
        return skip_unary(pp);
    default:
        return skip_postfix(pp);
    }
}

#define DEFINE_SKIP_BINOP(name, child, condition)              \
static bool name(ElPreproc* pp) {                              \
    if (!child(pp)) return false;                              \
    ElToken tok;                                               \
    while (_el_pp_peek(pp, &tok) && (condition)) {             \
        _el_pp_advance(pp);                                    \
        if (!child(pp)) return false;                          \
    }                                                          \
    return true;                                               \
}

// a very sloppy way of eliminating boilerplate (it works, tho)
// TODO: use a pratt parser or something
DEFINE_SKIP_BINOP(skip_multiplicative, skip_unary,          tok.type == EL_TT_STAR || tok.type == EL_TT_SLASH || tok.type == EL_TT_PERCENT)
DEFINE_SKIP_BINOP(skip_additive,       skip_multiplicative, tok.type == EL_TT_PLUS || tok.type == EL_TT_MINUS)
DEFINE_SKIP_BINOP(skip_shift,          skip_additive,       tok.type == EL_TT_SHL || tok.type == EL_TT_SHR)
DEFINE_SKIP_BINOP(skip_relational,     skip_shift,          tok.type == EL_TT_LT || tok.type == EL_TT_LTE || tok.type == EL_TT_GT || tok.type == EL_TT_GTE)
DEFINE_SKIP_BINOP(skip_equality,       skip_relational,     tok.type == EL_TT_EQL || tok.type == EL_TT_NEQ)
DEFINE_SKIP_BINOP(skip_bitwise_and,    skip_equality,       tok.type == EL_TT_BITWISE_AND)
DEFINE_SKIP_BINOP(skip_bitwise_xor,    skip_bitwise_and,    tok.type == EL_TT_BITWISE_XOR)
DEFINE_SKIP_BINOP(skip_bitwise_or,     skip_bitwise_xor,    tok.type == EL_TT_BITWISE_OR)
DEFINE_SKIP_BINOP(skip_bitwise_imp,    skip_bitwise_or,     tok.type == EL_TT_BITWISE_IMP)
DEFINE_SKIP_BINOP(skip_logical_and,    skip_bitwise_imp,    tok.type == EL_TT_LOGICAL_AND)
DEFINE_SKIP_BINOP(skip_logical_or,     skip_logical_and,    tok.type == EL_TT_LOGICAL_OR)

bool _el_pp_skip_expr(ElPreproc* pp) {
    if (!skip_logical_or(pp)) return false;
    while (true) {
        ElToken tok;
        if (!_el_pp_peek(pp, &tok) || tok.type != EL_TT_LOGICAL_IMP) break;
        _el_pp_advance(pp);
        if (!skip_logical_or(pp)) return false;
    }
    return true;
}
