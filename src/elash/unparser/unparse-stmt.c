#include <elash/unparser/unparser.h>

#include <elash/util/assert.h>
#include <elash/sema/bin-op.h>

static ElTokenType cassign_token(ElBinOp op) {
    switch (op) {
    case EL_BIN_OP_ADD:    return EL_TT_ADD_ASSIGN;
    case EL_BIN_OP_SUB:    return EL_TT_SUB_ASSIGN;
    case EL_BIN_OP_MUL:    return EL_TT_MUL_ASSIGN;
    case EL_BIN_OP_DIV:    return EL_TT_DIV_ASSIGN;
    case EL_BIN_OP_MOD:    return EL_TT_MOD_ASSIGN;
    case EL_BIN_OP_BW_AND: return EL_TT_BITWISE_AND_ASSIGN;
    case EL_BIN_OP_BW_OR:  return EL_TT_BITWISE_OR_ASSIGN;
    case EL_BIN_OP_BW_XOR: return EL_TT_BITWISE_XOR_ASSIGN;
    case EL_BIN_OP_AND:    return EL_TT_LOGICAL_AND_ASSIGN;
    case EL_BIN_OP_OR:     return EL_TT_LOGICAL_OR_ASSIGN;
    case EL_BIN_OP_IMP:    return EL_TT_LOGICAL_IMP_ASSIGN;
    case EL_BIN_OP_BW_IMP: return EL_TT_BITWISE_IMP_ASSIGN;
    case EL_BIN_OP_SHL:    return EL_TT_SHL_ASSIGN;
    case EL_BIN_OP_SHR:    return EL_TT_SHR_ASSIGN;
    default:
        EL_UNREACHABLE("invalid compound assignment op");
    }
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity): the logic is flat
void el_unparser_unparse_stmt(ElUnparser* unpar, ElAstStmt* stmt) {
    switch (stmt->type) {
    case EL_AST_STMT_EXPR:
        el_unparser_unparse_expr(unpar, stmt->as.expr);
        el_unparser_push_punct(unpar, EL_TT_SEMICOLON);
        return;

    case EL_AST_STMT_RETURN:
        el_unparser_push_kw(unpar, EL_TT_KW_RETURN);
        if (stmt->as.return_.value != NULL) {
            el_unparser_unparse_init(unpar, stmt->as.return_.value);
        }
        el_unparser_push_punct(unpar, EL_TT_SEMICOLON);
        return;

    case EL_AST_STMT_DECL:
        return el_unparser_unparse_decl(unpar, stmt->as.decl);

    case EL_AST_STMT_ASSIGN:
        el_unparser_unparse_expr(unpar, stmt->as.assign.target);
        el_unparser_push_punct(unpar, EL_TT_ASSIGN);
        el_unparser_unparse_init(unpar, stmt->as.assign.value);
        return el_unparser_push_punct(unpar, EL_TT_SEMICOLON);

    case EL_AST_STMT_CASSIGN:
        el_unparser_unparse_expr(unpar, stmt->as.cassign.target);
        el_unparser_push_punct(unpar, cassign_token(stmt->as.cassign.op));
        el_unparser_unparse_init(unpar, stmt->as.cassign.value);
        el_unparser_push_punct(unpar, EL_TT_SEMICOLON);
        return;

    case EL_AST_STMT_BLOCK:
        _el_unparser_unparse_block(unpar, &stmt->as.block);
        return;

    case EL_AST_STMT_IF:
        el_unparser_push_kw(unpar, EL_TT_KW_IF);
        el_unparser_push_punct(unpar, EL_TT_LPAREN);
        if (stmt->as.if_.init != NULL) {
            el_unparser_unparse_stmt(unpar, stmt->as.if_.init);
        }
        el_unparser_unparse_expr(unpar, stmt->as.if_.cond);
        el_unparser_push_punct(unpar, EL_TT_RPAREN);
        el_unparser_unparse_stmt(unpar, stmt->as.if_.then);
        if (stmt->as.if_.else_ != NULL) {
            el_unparser_push_kw(unpar, EL_TT_KW_ELSE);
            el_unparser_unparse_stmt(unpar, stmt->as.if_.else_);
        }
        return;

    case EL_AST_STMT_WHILE:
        el_unparser_push_kw(unpar, EL_TT_KW_WHILE);
        el_unparser_push_punct(unpar, EL_TT_LPAREN);
        if (stmt->as.while_.init != NULL) {
            el_unparser_unparse_stmt(unpar, stmt->as.while_.init);
        }

        el_unparser_unparse_expr(unpar, stmt->as.while_.cond);
        el_unparser_push_punct(unpar, EL_TT_RPAREN);
        el_unparser_unparse_stmt(unpar, stmt->as.while_.body);
        return;

    case EL_AST_STMT_BREAK:
        el_unparser_push_kw(unpar, EL_TT_KW_BREAK);
        el_unparser_push_punct(unpar, EL_TT_SEMICOLON);
        return;

    case EL_AST_STMT_CONTINUE:
        el_unparser_push_kw(unpar, EL_TT_KW_CONTINUE);
        el_unparser_push_punct(unpar, EL_TT_SEMICOLON);
        return;
    }
    EL_UNREACHABLE_ENUM_VAL(ElAstStmtType, stmt->type);
}
