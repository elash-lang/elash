#include <elash/parser/parser.h>

#include <elash/ast/tree/toe.h>
#include <elash/ast/tree/expr/bin.h>
#include <elash/ast/tree/expr/unary.h>

#include <elash/sema/bin-op.h>
#include <elash/sema/unary-op.h>

static bool is_type_literal(ElParser* parser) {
    usize idx = 0;
    if (!_el_parser_lookahead_skip_type(parser, &idx)) return false;
    return el_parser_peek_at(parser, idx).type == EL_TT_LBRACE;
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
    while (true) {
        if (el_parser_check(parser, EL_TT_INC)) {
            ElToken tok = el_parser_advance(parser);
            expr = el_ast_new_unary_expr(parser->arena, el_source_span_merge(expr->span, tok.span), EL_SEMA_UNARY_OP_POST_INC, expr);
        } else if (el_parser_check(parser, EL_TT_DEC)) {
            ElToken tok = el_parser_advance(parser);
            expr = el_ast_new_unary_expr(parser->arena, el_source_span_merge(expr->span, tok.span), EL_SEMA_UNARY_OP_POST_DEC, expr);
        } else if (el_parser_match(parser, EL_TT_LPAREN)) {
            expr = _el_parser_parse_call(parser, expr);
        } else if (el_parser_check(parser, EL_TT_CARET)) {
            ElToken tok = el_parser_advance(parser);
            expr = el_ast_new_unary_expr(parser->arena, el_source_span_merge(expr->span, tok.span), EL_SEMA_UNARY_OP_DEREF, expr);
        } else if (el_parser_match(parser, EL_TT_LBRACKET)) {
            ElAstExpr* index = el_parser_parse_expr(parser);
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
                parser->arena,
                el_source_span_merge(expr->span, rbracket.span),
                EL_SEMA_BIN_OP_INDEX, expr, index
            );
        } else if (el_parser_match(parser, EL_TT_DOT)) {
            expr = _el_parser_parse_member(parser, expr);
        } else {
            break;
        }

        if (el_parser_has_errs(parser)) return NULL;
    }
    return expr;
}

static ElAstToE* force_type_with_suffixes(ElParser* parser, ElAstToE* toe) {
    ElAstType* type = el_ast_toe_as_type(parser->arena, toe);
    if (type == NULL) return NULL;
    type = _el_parser_parse_type_suffixes(parser, type);
    if (type == NULL) return NULL;
    return el_ast_new_toe_type(parser->arena, type);
}

// i don't even want to think about how complicated this will get once we add function types
// ReturnType(ParamType1, ...)

static ElAstToE* parse_toe_bracket_suffix(ElParser* parser, ElAstToE* toe) {
    el_parser_advance(parser); // [

    ElAstToE* index = _el_parser_parse_type_or_expr(parser);
    if (index == NULL) return NULL;

    ElToken rbracket = parser->current;
    el_parser_expect(parser, EL_TT_RBRACKET);
    if (el_parser_has_errs(parser)) return NULL;

    ElSourceSpan combined_span = el_source_span_merge(toe->span, rbracket.span);

    if (toe->kind == EL_AST_TOE_TYPE) {
        ElAstExpr* size = el_ast_toe_as_expr(parser->arena, index);
        if (size == NULL) return NULL;

        ElAstType* type = el_ast_new_type_array(
            parser->arena, combined_span, toe->as.type, size
        );
        return el_ast_new_toe_type(parser->arena, type);
    }

    return el_ast_new_toe_unr_index(
        parser->arena, combined_span, toe, index
    );
}

static ElAstToE* parse_toe_suffixes(ElParser* parser, ElAstToE* toe) {
    while (toe != NULL) {
        // slices and refs
        if (el_parser_check(parser, EL_TT_BITWISE_AND)) {
            return force_type_with_suffixes(parser, toe);
        }
        if (el_parser_check(parser, EL_TT_LBRACKET) && is_slice_brackets(parser)) {
            return force_type_with_suffixes(parser, toe);
        }

        // X[123]
        // (can be an array type or an index expression)
        if (el_parser_check(parser, EL_TT_LBRACKET) && toe->kind != EL_AST_TOE_EXPR) {
            toe = parse_toe_bracket_suffix(parser, toe);
            if (toe == NULL) return NULL;
            continue;
        }

        if (el_parser_check(parser, EL_TT_INC)
         || el_parser_check(parser, EL_TT_DEC)
         || el_parser_check(parser, EL_TT_LPAREN)
         || el_parser_check(parser, EL_TT_CARET)
         || el_parser_check(parser, EL_TT_DOT)
         || (el_parser_check(parser, EL_TT_LBRACKET) && toe->kind == EL_AST_TOE_EXPR)
        ) {
            ElAstExpr* expr = el_ast_toe_as_expr(parser->arena, toe);
            if (expr == NULL) return NULL;
            expr = continue_expr_postfixes(parser, expr);
            if (expr == NULL) return NULL;
            return el_ast_new_toe_expr(parser->arena, expr);
        }

        break;
    }

    return toe;
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
static bool is_complex_expr(ElParser* parser) {
    // NOLINTNEXTLINE
    usize idx = 0, paren_depth = 0, bracket_depth = 0, brace_depth = 0;
    while (true) {
        ElToken tok = el_parser_peek_at(parser, idx);
        if (tok.type == EL_TT_EOF || tok.type == EL_TT_SEMICOLON)
            break;

        if (tok.type == EL_TT_LPAREN)
            paren_depth++;
        else if (tok.type == EL_TT_RPAREN)
            if (paren_depth > 0) paren_depth--;
            else break;
        else if (tok.type == EL_TT_LBRACKET)
            bracket_depth++;
        else if (tok.type == EL_TT_RBRACKET)
            if (bracket_depth > 0) bracket_depth--;
            else break;
        else if (tok.type == EL_TT_LBRACE)
            brace_depth++;
        else if (tok.type == EL_TT_RBRACE)
            if (brace_depth > 0) brace_depth--;
            else break;
        else if (paren_depth == 0 && bracket_depth == 0 && brace_depth == 0)
            if (is_binary_op_or_cast(parser, idx))
                return true;
        idx++;
    }
    return false;
}

ElAstToE* _el_parser_parse_type_or_expr(ElParser* parser) {
    if (is_complex_expr(parser)) {
        ElAstExpr* expr = el_parser_parse_expr(parser);
        if (expr == NULL) return NULL;
        return el_ast_new_toe_expr(parser->arena, expr);
    }

    // unambiguous cases

    // Type { ... }
    if (is_type_literal(parser)) {
        ElAstExpr* expr = _el_parser_parse_postfix(parser);
        if (expr == NULL) return NULL;
        return el_ast_new_toe_expr(parser->arena, expr);
    }

    // struct(...) / struct { ... }
    if (el_parser_check(parser, EL_TT_KW_STRUCT)) {
        ElAstType* type = _el_parser_parse_type(parser);
        if (type == NULL) return NULL;
        return el_ast_new_toe_type(parser->arena, type);
    }

    // ambiguous
    // X (just a identifier)
    if (el_parser_check(parser, EL_TT_IDENT)) {
        ElAstIdent* ident = _el_parser_parse_ident(parser);
        if (ident == NULL) return NULL;

        ElAstToE* toe = el_ast_new_toe_unr_ident(parser->arena, ident->span, ident);
        return parse_toe_suffixes(parser, toe);
    }

    ElAstExpr* expr = el_parser_parse_expr(parser);
    if (expr == NULL) return NULL;
    return el_ast_new_toe_expr(parser->arena, expr);
}
