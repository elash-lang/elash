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
bool el_unparser_unparse_stmt(ElUnparser* unpar, ElAstStmt* stmt) {
    switch (stmt->type) {
    case EL_AST_STMT_EXPR:
        if (!el_unparser_unparse_expr(unpar, stmt->as.expr)) return false;
        return el_unparser_push_punct(unpar, EL_TT_SEMICOLON);

    case EL_AST_STMT_RETURN:
        if (!el_unparser_push_kw(unpar, EL_TT_KW_RETURN)) return false;
        if (stmt->as.return_.value != NULL) {
            if (!el_unparser_unparse_init(unpar, stmt->as.return_.value)) return false;
        }
        return el_unparser_push_punct(unpar, EL_TT_SEMICOLON);

    case EL_AST_STMT_DECL:
        return el_unparser_unparse_decl(unpar, stmt->as.decl);

    case EL_AST_STMT_ASSIGN:
        if (!el_unparser_unparse_expr(unpar, stmt->as.assign.target)) return false;
        if (!el_unparser_push_punct(unpar, EL_TT_ASSIGN))             return false;
        if (!el_unparser_unparse_init(unpar, stmt->as.assign.value))  return false;
        return el_unparser_push_punct(unpar, EL_TT_SEMICOLON);

    case EL_AST_STMT_COMPOUND_ASSIGN:
        if (!el_unparser_unparse_expr(unpar, stmt->as.cassign.target))          return false;
        if (!el_unparser_push_punct(unpar, cassign_token(stmt->as.cassign.op))) return false;
        if (!el_unparser_unparse_init(unpar, stmt->as.cassign.value))           return false;
        return el_unparser_push_punct(unpar, EL_TT_SEMICOLON);

    case EL_AST_STMT_BLOCK:
        return _el_unparser_unparse_block(unpar, &stmt->as.block);

    case EL_AST_STMT_IF:
        if (!el_unparser_push_kw(unpar, EL_TT_KW_IF))            return false;
        if (!el_unparser_push_punct(unpar, EL_TT_LPAREN))        return false;
        if (stmt->as.if_.init != NULL) {
            if (!el_unparser_unparse_stmt(unpar, stmt->as.if_.init)) return false;
        }
        if (!el_unparser_unparse_expr(unpar, stmt->as.if_.cond)) return false;
        if (!el_unparser_push_punct(unpar, EL_TT_RPAREN))        return false;
        if (!el_unparser_unparse_stmt(unpar, stmt->as.if_.then)) return false;
        if (stmt->as.if_.else_ != NULL) {
            if (!el_unparser_push_kw(unpar, EL_TT_KW_ELSE))           return false;
            if (!el_unparser_unparse_stmt(unpar, stmt->as.if_.else_)) return false;
        }
        return true;

    case EL_AST_STMT_WHILE:
        if (!el_unparser_push_kw(unpar, EL_TT_KW_WHILE))            return false;
        if (!el_unparser_push_punct(unpar, EL_TT_LPAREN))           return false;
        if (stmt->as.while_.init != NULL) {
            if (!el_unparser_unparse_stmt(unpar, stmt->as.while_.init)) return false;
        }
        if (!el_unparser_unparse_expr(unpar, stmt->as.while_.cond)) return false;
        if (!el_unparser_push_punct(unpar, EL_TT_RPAREN))           return false;
        return el_unparser_unparse_stmt(unpar, stmt->as.while_.body);

    case EL_AST_STMT_BREAK:
        if (!el_unparser_push_kw(unpar, EL_TT_KW_BREAK)) return false;
        return el_unparser_push_punct(unpar, EL_TT_SEMICOLON);

    case EL_AST_STMT_CONTINUE:
        if (!el_unparser_push_kw(unpar, EL_TT_KW_CONTINUE)) return false;
        return el_unparser_push_punct(unpar, EL_TT_SEMICOLON);
    }
    EL_UNREACHABLE_ENUM_VAL(ElAstStmtType, stmt->type);
}
