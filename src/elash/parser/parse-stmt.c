#include <elash/parser/parser.h>

#include <elash/diag/engine.h>

#include <elash/ast/tree/stmt.h>
#include <elash/ast/tree/stmt/return.h>
#include <elash/ast/tree/stmt/if.h>
#include <elash/ast/tree/stmt/assign.h>
#include <elash/ast/tree/stmt/cassign.h>

ElAstStmt* _el_parser_parse_return(ElParser* parser, ElToken return_tok) {
    if (el_parser_check(parser, EL_TT_SEMICOLON)) {
        ElToken semi_tok = el_parser_advance(parser);
        return el_ast_new_return_stmt(parser->arena, el_source_span_merge(return_tok.span, semi_tok.span), NULL);
    }

    ElAstExpr* expr = el_parser_parse_expr(parser);
    if (el_parser_has_errs(parser)) return NULL;

    ElToken semi_tok = el_parser_expect(parser, EL_TT_SEMICOLON);
    if (el_parser_has_errs(parser)) return NULL;

    return el_ast_new_return_stmt(parser->arena, el_source_span_merge(return_tok.span, semi_tok.span), expr);
}

ElAstStmt* _el_parser_parse_if(ElParser* parser, ElToken if_tok) {
    ElSourceSpan end_span;

    el_parser_expect(parser, EL_TT_LPAREN);
    if (el_parser_has_errs(parser))
        return el_parser_sync(parser, EL_PARSER_SYNC_STMT);

    ElAstExpr* cond = el_parser_parse_expr(parser);
    if (el_parser_has_errs(parser))
        return el_parser_sync(parser, EL_PARSER_SYNC_STMT);

    if (!el_parser_check(parser, EL_TT_RPAREN)) {
        el_parser_expect(parser, EL_TT_RPAREN);
        el_parser_skip_to(parser, EL_TT_RPAREN);
        if (el_parser_check(parser, EL_TT_RPAREN)) {
            el_parser_advance(parser);
        }
    } else {
        el_parser_advance(parser);
    }

    ElAstStmt* then_stmt = el_parser_parse_stmt(parser);
    if (el_parser_has_errs(parser)) return NULL;

    end_span = then_stmt->span;

    ElAstStmt* else_stmt = NULL;
    if (el_parser_match(parser, EL_TT_KW_ELSE)) {
        else_stmt = el_parser_parse_stmt(parser);
        if (el_parser_has_errs(parser)) return NULL;
        end_span = else_stmt->span;
    }

    return el_ast_new_if_stmt(
        parser->arena,
        el_source_span_merge(if_tok.span, end_span),
        cond, then_stmt, else_stmt
    );
}

static ElAstStmt* _el_parser_parse_while(ElParser* parser, ElToken while_tok) {
    el_parser_expect(parser, EL_TT_LPAREN);
    if (el_parser_has_errs(parser))
        return el_parser_sync(parser, EL_PARSER_SYNC_STMT);

    ElAstExpr* cond = el_parser_parse_expr(parser);
    if (el_parser_has_errs(parser))
        return el_parser_sync(parser, EL_PARSER_SYNC_STMT);

    if (!el_parser_check(parser, EL_TT_RPAREN)) {
        el_parser_expect(parser, EL_TT_RPAREN);
        el_parser_skip_to(parser, EL_TT_RPAREN);
        if (el_parser_check(parser, EL_TT_RPAREN)) {
            el_parser_advance(parser);
        }
    } else {
        el_parser_advance(parser);
    }

    ElAstStmt* body_stmt = el_parser_parse_stmt(parser);
    if (el_parser_has_errs(parser)) return NULL;

    return el_ast_new_while_stmt(
        parser->arena,
        el_source_span_merge(while_tok.span, body_stmt->span),
        cond, body_stmt
    );
}

ElAstStmt* _el_parser_parse_expr_stmt(ElParser* parser) {
    ElAstExpr* expr = el_parser_parse_expr(parser);
    if (expr == NULL) return NULL;

    if (el_parser_match(parser, EL_TT_ASSIGN)) {
        ElAstInit* value = el_parser_parse_init(parser);
        if (value == NULL) return NULL;

        ElToken semi_tok = el_parser_expect(parser, EL_TT_SEMICOLON);
        if (el_parser_has_errs(parser)) return NULL;

        return el_ast_new_assign_stmt(parser->arena, el_source_span_merge(expr->span, semi_tok.span), expr, value);
    }

    ElSemaBinOp op;
    bool is_compound = false;
    if      (el_parser_match(parser, EL_TT_ADD_ASSIGN))         { op = EL_SEMA_BIN_OP_ADD;    is_compound = true; }
    else if (el_parser_match(parser, EL_TT_SUB_ASSIGN))         { op = EL_SEMA_BIN_OP_SUB;    is_compound = true; }
    else if (el_parser_match(parser, EL_TT_MUL_ASSIGN))         { op = EL_SEMA_BIN_OP_MUL;    is_compound = true; }
    else if (el_parser_match(parser, EL_TT_DIV_ASSIGN))         { op = EL_SEMA_BIN_OP_DIV;    is_compound = true; }
    else if (el_parser_match(parser, EL_TT_MOD_ASSIGN))         { op = EL_SEMA_BIN_OP_MOD;    is_compound = true; }
    else if (el_parser_match(parser, EL_TT_BITWISE_AND_ASSIGN)) { op = EL_SEMA_BIN_OP_BW_AND; is_compound = true; }
    else if (el_parser_match(parser, EL_TT_BITWISE_OR_ASSIGN))  { op = EL_SEMA_BIN_OP_BW_OR;  is_compound = true; }
    else if (el_parser_match(parser, EL_TT_BITWISE_XOR_ASSIGN)) { op = EL_SEMA_BIN_OP_BW_XOR; is_compound = true; }
    else if (el_parser_match(parser, EL_TT_LOGICAL_AND_ASSIGN)) { op = EL_SEMA_BIN_OP_AND;    is_compound = true; }
    else if (el_parser_match(parser, EL_TT_LOGICAL_OR_ASSIGN))  { op = EL_SEMA_BIN_OP_OR;     is_compound = true; }
    else if (el_parser_match(parser, EL_TT_LOGICAL_IMP_ASSIGN)) { op = EL_SEMA_BIN_OP_IMP;    is_compound = true; }
    else if (el_parser_match(parser, EL_TT_BITWISE_IMP_ASSIGN)) { op = EL_SEMA_BIN_OP_BW_IMP; is_compound = true; }
    else if (el_parser_match(parser, EL_TT_SHL_ASSIGN))         { op = EL_SEMA_BIN_OP_SHL;    is_compound = true; }
    else if (el_parser_match(parser, EL_TT_SHR_ASSIGN))         { op = EL_SEMA_BIN_OP_SHR;    is_compound = true; }

    if (is_compound) {
        ElAstInit* value = el_parser_parse_init(parser);
        if (value == NULL) return NULL;

        ElToken semi_tok = el_parser_expect(parser, EL_TT_SEMICOLON);
        if (el_parser_has_errs(parser)) {
            return NULL;
        }

        return el_ast_new_compound_assign_stmt(parser->arena, el_source_span_merge(expr->span, semi_tok.span), op, expr, value);
    }

    ElToken semi_tok = el_parser_expect(parser, EL_TT_SEMICOLON);
    if (el_parser_has_errs(parser)) {
        return NULL;
    }

    return el_ast_new_expr_stmt(parser->arena, el_source_span_merge(expr->span, semi_tok.span), expr);
}

ElAstStmt* _el_parser_parse_block(ElParser* parser, ElToken lbrace_tok) {
    ElAstStmt* head = NULL;
    ElAstStmt* tail = NULL;

    while (parser->current.type != EL_TT_RBRACE && parser->current.type != EL_TT_EOF) {
        ElAstStmt* stmt = el_parser_parse_stmt(parser);
        if (el_parser_has_errs(parser)) {
            el_parser_sync(parser, EL_PARSER_SYNC_STMT);
            continue;
        }

        if (stmt) {
            el_ast_stmt_list_append(&head, &tail, stmt);
        }
    }

    ElToken rbrace_tok = parser->current;
    if (el_parser_check(parser, EL_TT_RBRACE)) {
        el_parser_advance(parser);
    } else {
        el_parser_expect(parser, EL_TT_RBRACE);
        el_parser_sync(parser, EL_PARSER_SYNC_BLOCK);
        if (el_parser_check(parser, EL_TT_RBRACE)) {
            rbrace_tok = el_parser_advance(parser);
        }
    }

    return el_ast_new_block_stmt(parser->arena, el_source_span_merge(lbrace_tok.span, rbrace_tok.span), head);
}

ElAstStmt* el_parser_parse_stmt(ElParser* parser) {
    if (el_parser_check(parser, EL_TT_KW_RETURN)) {
        ElToken return_tok = el_parser_advance(parser);
        return _el_parser_parse_return(parser, return_tok);
    }
    if (el_parser_check(parser, EL_TT_KW_IF)) {
        ElToken if_tok = el_parser_advance(parser);
        return _el_parser_parse_if(parser, if_tok);
    }
    if (el_parser_check(parser, EL_TT_KW_WHILE)) {
        ElToken while_tok = el_parser_advance(parser);
        return _el_parser_parse_while(parser, while_tok);
    }
    if (el_parser_check(parser, EL_TT_LBRACE)) {
        ElToken lbrace_tok = el_parser_advance(parser);
        return _el_parser_parse_block(parser, lbrace_tok);
    }

    if (el_parser_check(parser, EL_TT_KW_BREAK)) {
        ElToken break_tok = el_parser_advance(parser);
        ElToken semi_tok = el_parser_expect(parser, EL_TT_SEMICOLON);
        return el_ast_new_break_stmt(parser->arena, el_source_span_merge(break_tok.span, semi_tok.span));
    }
    if (el_parser_check(parser, EL_TT_KW_CONTINUE)) {
        ElToken continue_tok = el_parser_advance(parser);
        ElToken semi_tok = el_parser_expect(parser, EL_TT_SEMICOLON);
        return el_ast_new_continue_stmt(parser->arena, el_source_span_merge(continue_tok.span, semi_tok.span));
    }

    usize lookahead_idx = 0;
    bool is_decl = el_parser_check(parser, EL_TT_KW_EXTERN);
    if (!is_decl && _el_parser_lookahead_skip_type(parser, &lookahead_idx)) {
        is_decl = el_parser_peek_at(parser, lookahead_idx).type == EL_TT_IDENT;
    }

    if (is_decl) {
        ElAstDecl* decl = el_parser_parse_decl(parser);
        if (decl == NULL) return NULL;
        return el_ast_new_decl_stmt(parser->arena, decl->span, decl);
    }

    return _el_parser_parse_expr_stmt(parser);
}
