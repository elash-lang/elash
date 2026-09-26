#include "parser-internals.h"

#include <elash/ast/tree/unr.h>
#include <elash/ast/tree/expr/bin.h>
#include <elash/ast/tree/expr/unary.h>

#include <elash/sema/bin-op.h>
#include <elash/sema/unary-op.h>
#include <elash/util/assert.h>

static ElParseAmbig ambig_type(ElAstType* type) {
    return (ElParseAmbig){ .kind = EL_PARSE_AMBIG_TYPE, .span = type->span, .as.type = type };
}

static ElParseAmbig ambig_expr(ElAstExpr* expr) {
    return (ElParseAmbig){ .kind = EL_PARSE_AMBIG_EXPR, .span = expr->span, .as.expr = expr };
}

static ElParseAmbig ambig_unr(ElAstUnr* unr) {
    return (ElParseAmbig){ .kind = EL_PARSE_AMBIG_UNR, .span = unr->span, .as.unr = unr };
}

static ElAstExpr* ambig_as_expr(ElDynArena* arena, ElParseAmbig node) {
    switch (node.kind) {
    case EL_PARSE_AMBIG_EXPR:
        return node.as.expr;
    case EL_PARSE_AMBIG_UNR:
        return el_ast_unr_as_expr(arena, node.as.unr);
    case EL_PARSE_AMBIG_TYPE:
        return NULL;
    }
    EL_UNREACHABLE_ENUM_VAL(ElParseAmbigKind, node.kind);
}

static bool is_slice_brackets(ElParser* parser) {
    ElToken inner = el_parser_peek_at(parser, 1);
    if (inner.type == EL_TT_RBRACKET) return true;
    if (inner.type == EL_TT_BITWISE_AND && el_parser_peek_at(parser, 2).type == EL_TT_RBRACKET) {
        return true;
    }
    return false;
}

static ElAstExpr* continue_expr_postfixes(ElParser* parser, ElAstExpr* expr) {
    return _el_parse_continue_postfixes(parser, expr);
}

static ElParseAmbig force_type_with_suffixes(ElParser* parser, ElParseAmbig node) {
    ElAstType* type = NULL;
    switch (node.kind) {
    case EL_PARSE_AMBIG_TYPE:
        type = node.as.type;
        break;
    case EL_PARSE_AMBIG_UNR:
        type = el_ast_unr_as_type(parser->aarena, node.as.unr);
        break;
    case EL_PARSE_AMBIG_EXPR:
        return node;
    }
    if (type == NULL) return node;

    type = _el_parse_type_mut_and_suffixes(parser, type);
    if (type == NULL) return node;
    return ambig_type(type);
}

// i don't even want to think about how complicated this will get once we add function types
// ReturnType(ParamType1, ...)

static ElParseAmbig parse_ambig_bracket_suffix(ElParser* parser, ElParseAmbig base) {
    el_parser_advance(parser); // [

    ElAstExpr* index_expr = el_parse_expr(parser);

    ElToken rbracket = parser->current;
    el_parser_expect(parser, EL_TT_RBRACKET);
    if (el_parser_has_errs(parser)) return base;

    ElSourceSpan combined_span = el_srcspan_merge(base.span, rbracket.span);

    if (base.kind == EL_PARSE_AMBIG_TYPE) {
        ElAstType* type = el_ast_new_type_array(
            parser->aarena, combined_span, EL_MUTSPEC_DEFAULT, base.as.type, index_expr
        );
        return ambig_type(type);
    }

    EL_ASSERT(base.kind == EL_PARSE_AMBIG_UNR, "bracket suffix base must be unresolved");

    return ambig_unr(el_ast_new_unr_index(
        parser->aarena, combined_span, base.as.unr, NULL, index_expr
    ));
}

static ElParseAmbig parse_ambig_suffixes(ElParser* parser, ElParseAmbig node) {
    while (true) {
        // slices and refs and optional types, or mutability specifiers
        if (el_parser_check(parser, EL_TT_BITWISE_AND)
         || el_parser_check(parser, EL_TT_OPT)
         || el_parser_check(parser, EL_TT_KW_CONST)
         || el_parser_check(parser, EL_TT_KW_WONLY)
        ) {
            return force_type_with_suffixes(parser, node);
        }
        if (el_parser_check(parser, EL_TT_LBRACKET) && is_slice_brackets(parser)) {
            return force_type_with_suffixes(parser, node);
        }

        // X[123]
        // (can be an array type or an index expression)
        if (el_parser_check(parser, EL_TT_LBRACKET) && node.kind != EL_PARSE_AMBIG_EXPR) {
            node = parse_ambig_bracket_suffix(parser, node);
            if (node.kind == EL_PARSE_AMBIG_TYPE && node.as.type == NULL) return node;
            continue;
        }

        if (el_parser_check(parser, EL_TT_INC)
         || el_parser_check(parser, EL_TT_DEC)
         || el_parser_check(parser, EL_TT_LPAREN)
         || el_parser_check(parser, EL_TT_CARET)
         || el_parser_check(parser, EL_TT_DOT)
         || (el_parser_check(parser, EL_TT_LBRACKET) && node.kind == EL_PARSE_AMBIG_EXPR)
        ) {
            ElAstExpr* expr = ambig_as_expr(parser->aarena, node);
            if (expr == NULL) return node;
            expr = continue_expr_postfixes(parser, expr);
            if (expr == NULL) return node;
            return ambig_expr(expr);
        }

        break;
    }

    return node;
}

static bool is_binary_op_or_cast(ElParser* parser, usize idx) {
    ElToken tok = el_parser_peek_at(parser, idx);
    switch (tok.type) {
    case EL_TT_KW_AS: case EL_TT_PLUS: case EL_TT_MINUS: case EL_TT_STAR: case EL_TT_SLASH: case EL_TT_PERCENT:
    case EL_TT_LOGICAL_AND: case EL_TT_LOGICAL_OR: case EL_TT_LOGICAL_IMP: case EL_TT_BITWISE_IMP:
    case EL_TT_EQL: case EL_TT_NEQ: case EL_TT_LT: case EL_TT_GT: case EL_TT_LTE: case EL_TT_GTE:
        return true;
    case EL_TT_BITWISE_AND: {
        ElToken next = el_parser_peek_at(parser, idx + 1);
        if (next.type == EL_TT_LBRACKET || next.type == EL_TT_BITWISE_AND || next.type == EL_TT_OPT || is_mut_spec_token(next.type)
        ) {
            return false;
        }

        if (next.type != EL_TT_SEMICOLON &&
            next.type != EL_TT_COMMA &&
            next.type != EL_TT_RBRACKET &&
            next.type != EL_TT_RBRACE &&
            next.type != EL_TT_RPAREN &&
            next.type != EL_TT_EOF) {
            return true;
        }
        return false;
    }
    default:
        return false;
    }
}

// NOLINTNEXTLINE
bool _el_parser_is_complex_expr(ElParser* parser) {
    // NOLINTNEXTLINE
    usize idx = 0, paren_depth = 0, bracket_depth = 0, brace_depth = 0;
    while (true) {
        ElToken tok = el_parser_peek_at(parser, idx);
        if (tok.type == EL_TT_EOF)
            break;

        if (tok.type == EL_TT_LPAREN) {
            paren_depth++;
        } else if (tok.type == EL_TT_RPAREN) {
            if (paren_depth > 0) paren_depth--;
            else break;
        } else if (tok.type == EL_TT_LBRACKET) {
            bracket_depth++;
        } else if (tok.type == EL_TT_RBRACKET) {
            if (bracket_depth > 0) bracket_depth--;
            else break;
        } else if (tok.type == EL_TT_LBRACE) {
            brace_depth++;
        } else if (tok.type == EL_TT_RBRACE) {
            if (brace_depth > 0) brace_depth--;
            else break;
        } else if (tok.type == EL_TT_SEMICOLON) {
            if (paren_depth == 0 && bracket_depth == 0 && brace_depth == 0)
                break;
        } else if (paren_depth == 0 && bracket_depth == 0 && brace_depth == 0) {
            if (is_binary_op_or_cast(parser, idx))
                return true;
        }

        idx++;
    }
    return false;
}

ElParseAmbig _el_parse_ambig(ElParser* parser) {
    if (_el_parser_is_type_literal(parser)) {
        ElAstExpr* expr = _el_parse_postfix(parser);
        if (expr == NULL) {
            return (ElParseAmbig) { .kind = EL_PARSE_AMBIG_EXPR, .as.expr = NULL };
        }
        return ambig_expr(expr);
    }

    if (el_parser_check(parser, EL_TT_KW_CONST) || el_parser_check(parser, EL_TT_KW_WONLY)) {
        ElAstType* type = _el_parse_type(parser);
        if (type == NULL) {
            return (ElParseAmbig){ .kind = EL_PARSE_AMBIG_EXPR, .as.expr = NULL };
        }
        return ambig_type(type);
    }

    if (el_parser_check(parser, EL_TT_KW_STRUCT)) {
        ElAstType* type = _el_parse_type(parser);
        if (type == NULL) {
            return (ElParseAmbig){ .kind = EL_PARSE_AMBIG_EXPR, .as.expr = NULL };
        }
        return ambig_type(type);
    }

    if (el_parser_check(parser, EL_TT_IDENT)) {
        ElAstIdent* ident = _el_parse_ident(parser);
        if (ident == NULL) {
            return (ElParseAmbig){ .kind = EL_PARSE_AMBIG_EXPR, .as.expr = NULL };
        }

        ElParseAmbig node = ambig_unr(el_ast_new_unr_ident(parser->aarena, ident->span, ident));
        return parse_ambig_suffixes(parser, node);
    }

    ElAstExpr* expr = el_parse_expr(parser);
    if (expr == NULL) {
        return (ElParseAmbig){ .kind = EL_PARSE_AMBIG_EXPR, .as.expr = NULL };
    }
    return ambig_expr(expr);
}

ElAstToE* _el_parser_toe_from_ambig(ElParser* parser, ElParseAmbig node) {
    switch (node.kind) {
    case EL_PARSE_AMBIG_TYPE:
        return el_ast_new_toe_type(parser->aarena, node.as.type);
    case EL_PARSE_AMBIG_EXPR:
        return el_ast_new_toe_expr(parser->aarena, node.as.expr);
    case EL_PARSE_AMBIG_UNR:
        return el_ast_new_toe_unr(parser->aarena, node.as.unr);
    }
    EL_UNREACHABLE_ENUM_VAL(ElParseAmbigKind, node.kind);
}
