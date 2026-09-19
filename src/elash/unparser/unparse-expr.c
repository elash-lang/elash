#include <elash/unparser/unparser.h>

#include <elash/util/assert.h>
#include <elash/util/int128.h>
#include <elash/sema/bin-op.h>
#include <elash/sema/unary-op.h>
#include <elash/sema/storage-cls.h>

#include <inttypes.h>

enum {
    PREC_NONE     = 0,
    PREC_OPT      = 1,  // ?? ?>
    PREC_IMP      = 2,  // =>
    PREC_OR       = 3,  // ||
    PREC_AND      = 4,  // &&
    PREC_BW_IMP   = 5,  // ~>
    PREC_BW_OR    = 6,  // |
    PREC_BW_XOR   = 7,  // <>
    PREC_BW_AND   = 8,  // &
    PREC_EQ       = 9,  // == !=
    PREC_REL      = 10, // < <= > >=
    PREC_SHIFT    = 11, // << >>
    PREC_ADD      = 12, // + -
    PREC_MUL      = 13, // * / %
    PREC_CAST     = 14, // as
    PREC_UNARY    = 15, // guess
    PREC_POSTFIX  = 16, // also this
    PREC_PRIMARY  = 17, // this too
};

static int bin_op_prec(ElAstBinOp op) {
    switch (op) {
    case EL_SEMA_BIN_OP_OPT_FB:
    case EL_SEMA_BIN_OP_OPT_MAP: return PREC_OPT;
    case EL_SEMA_BIN_OP_IMP:    return PREC_IMP;
    case EL_SEMA_BIN_OP_OR:     return PREC_OR;
    case EL_SEMA_BIN_OP_AND:    return PREC_AND;
    case EL_SEMA_BIN_OP_BW_IMP: return PREC_BW_IMP;
    case EL_SEMA_BIN_OP_BW_OR:  return PREC_BW_OR;
    case EL_SEMA_BIN_OP_BW_XOR: return PREC_BW_XOR;
    case EL_SEMA_BIN_OP_BW_AND: return PREC_BW_AND;
    case EL_SEMA_BIN_OP_EQ:
    case EL_SEMA_BIN_OP_NEQ:    return PREC_EQ;
    case EL_SEMA_BIN_OP_LT:
    case EL_SEMA_BIN_OP_LTE:
    case EL_SEMA_BIN_OP_GT:
    case EL_SEMA_BIN_OP_GTE:    return PREC_REL;
    case EL_SEMA_BIN_OP_SHL:
    case EL_SEMA_BIN_OP_SHR:    return PREC_SHIFT;
    case EL_SEMA_BIN_OP_ADD:
    case EL_SEMA_BIN_OP_SUB:    return PREC_ADD;
    case EL_SEMA_BIN_OP_MUL:
    case EL_SEMA_BIN_OP_DIV:
    case EL_SEMA_BIN_OP_MOD:    return PREC_MUL;
    case EL_SEMA_BIN_OP_INDEX:  return PREC_POSTFIX;
    }
    EL_UNREACHABLE_ENUM_VAL(ElAstBinOp, op);
}

static ElTokenType bin_op_token(ElAstBinOp op) {
    switch (op) {
    case EL_SEMA_BIN_OP_ADD:     return EL_TT_PLUS;
    case EL_SEMA_BIN_OP_SUB:     return EL_TT_MINUS;
    case EL_SEMA_BIN_OP_MUL:     return EL_TT_STAR;
    case EL_SEMA_BIN_OP_DIV:     return EL_TT_SLASH;
    case EL_SEMA_BIN_OP_MOD:     return EL_TT_PERCENT;
    case EL_SEMA_BIN_OP_EQ:      return EL_TT_EQL;
    case EL_SEMA_BIN_OP_NEQ:     return EL_TT_NEQ;
    case EL_SEMA_BIN_OP_LT:      return EL_TT_LT;
    case EL_SEMA_BIN_OP_LTE:     return EL_TT_LTE;
    case EL_SEMA_BIN_OP_GT:      return EL_TT_GT;
    case EL_SEMA_BIN_OP_GTE:     return EL_TT_GTE;
    case EL_SEMA_BIN_OP_AND:     return EL_TT_LOGICAL_AND;
    case EL_SEMA_BIN_OP_OR:      return EL_TT_LOGICAL_OR;
    case EL_SEMA_BIN_OP_IMP:     return EL_TT_LOGICAL_IMP;
    case EL_SEMA_BIN_OP_OPT_FB:  return EL_TT_OPT_FB;
    case EL_SEMA_BIN_OP_OPT_MAP: return EL_TT_OPT_MAP;
    case EL_SEMA_BIN_OP_BW_AND:  return EL_TT_BITWISE_AND;
    case EL_SEMA_BIN_OP_BW_OR:   return EL_TT_BITWISE_OR;
    case EL_SEMA_BIN_OP_BW_XOR:  return EL_TT_BITWISE_XOR;
    case EL_SEMA_BIN_OP_BW_IMP:  return EL_TT_BITWISE_IMP;
    case EL_SEMA_BIN_OP_SHL:     return EL_TT_SHL;
    case EL_SEMA_BIN_OP_SHR:     return EL_TT_SHR;
    case EL_SEMA_BIN_OP_INDEX:   EL_UNREACHABLE("index is not a single token");
    }
    EL_UNREACHABLE_ENUM_VAL(ElAstBinOp, op);
}

static int expr_prec(ElAstExpr* expr) {
    switch (expr->type) {
    case EL_AST_EXPR_BINARY:
        return bin_op_prec(expr->as.binary.op);
    case EL_AST_EXPR_UNARY:
        if (el_unary_op_is_post(expr->as.unary.op)
            || expr->as.unary.op == EL_SEMA_UNARY_OP_DEREF) {
            return PREC_POSTFIX;
        }
        return PREC_UNARY;
    case EL_AST_EXPR_CAST:
        return PREC_CAST;
    case EL_AST_EXPR_CALL:
    case EL_AST_EXPR_MEMBER:
    case EL_AST_EXPR_TMEMBER:
        return PREC_POSTFIX;
    case EL_AST_EXPR_LITERAL:
    case EL_AST_EXPR_IDENT:
    case EL_AST_EXPR_TYPEDINIT:
        return PREC_PRIMARY;
    }
    EL_UNREACHABLE_ENUM_VAL(ElAstExprType, expr->type);
}

static bool unparse_paren_expr(ElUnparser* unpar, ElAstExpr* expr) {
    if (!el_unparser_push_punct(unpar, EL_TT_LPAREN)) return false;
    if (!el_unparser_unparse_expr(unpar, expr))       return false;
    return el_unparser_push_punct(unpar, EL_TT_RPAREN);
}

static bool unparse_child(ElUnparser* unpar, ElAstExpr* child, int parent_prec, bool is_right) {
    int child_p = expr_prec(child);
    bool need_paren = is_right ? (child_p <= parent_prec) : (child_p < parent_prec);
    if (need_paren) return unparse_paren_expr(unpar, child);
    return el_unparser_unparse_expr(unpar, child);
}

#define LITERAL_BUFSIZE 2038
static bool push_escapeified(ElUnparser* unpar, ElTokenType type, ElStringView sv) {
    char stack_buf[LITERAL_BUFSIZE];

    // the worst case, a string that contains only quotes or backslashes.
    // theoretically we could first count characters that actually need escaping
    // but RAM is cheaper than CPU time.
    usize needed = sv.len * 2;
    char* buf = (needed <= LITERAL_BUFSIZE)
        ? stack_buf : malloc(needed);

    if (buf == NULL)
        return false;

    usize bidx = 0;
    for (usize i = 0; i < sv.len; ++i) {
        // we don;t actually need to handle all escapes here. the lexer handles most special characters just fine
        // inside string/char literals, so we only need to ensure that we don't insert literal quotes or back slashes
        // (escaping single quotes in string literals / double quotes in char literals is fine.)
        switch (sv.data[i]) {
        case '\\': buf[bidx++] = '\\'; buf[bidx++] = '\\'; break;
        case '\"': buf[bidx++] = '\\'; buf[bidx++] = '\"'; break;
        case '\'': buf[bidx++] = '\\'; buf[bidx++] = '\''; break;
        default:
            buf[bidx++] = sv.data[i];
            break;
        }
    }

    bool success = el_unparser_push(unpar, type, el_sv_from_data_and_len(buf, bidx));
    if (buf != stack_buf) free(buf);
    return success;
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
static bool unparse_literal(ElUnparser* unpar, ElAstExpr* expr) {
    ElAstLiteral* lit = &expr->as.literal;
    switch (lit->kind) {
    case EL_AST_LIT_INT: {
        // NOLINTBEGIN(readability-magic-numbers)
        char buf[42];
        ElStringView str = el_i128_to_string(lit->of.int_, 10, buf);
        // NOLINTEND(readability-magic-numbers)
        return el_unparser_push(unpar, EL_TT_INT_LITERAL, str);
    }
    case EL_AST_LIT_FLOAT:
        return el_unparser_push_fmt(unpar, EL_TT_FLOAT_LITERAL, "%.17g", lit->of.float_);
    case EL_AST_LIT_CHAR:
        return push_escapeified(unpar, EL_TT_CHAR_LITERAL, el_sv_from_data_and_len(&lit->of.char_, 1));
    case EL_AST_LIT_STRING:
        return push_escapeified(unpar, EL_TT_STRING_LITERAL, lit->of.str_);
    case EL_AST_LIT_BOOL:
        return el_unparser_push_kw(
            unpar,
            lit->of.bool_ ? EL_TT_TRUE_LITERAL : EL_TT_FALSE_LITERAL
        );
    case EL_AST_LIT_NULL:
        return el_unparser_push_kw(unpar, EL_TT_NULL_LITERAL);
    }
    EL_UNREACHABLE_ENUM_VAL(ElAstLiteralKind, lit->kind);
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
static bool unparse_unary(ElUnparser* unpar, ElAstExpr* expr) {
    ElAstUnaryOp op = expr->as.unary.op;
    ElAstExpr* operand = expr->as.unary.operand;
    int prec = expr_prec(expr);

    if (el_unary_op_is_post(op) || op == EL_SEMA_UNARY_OP_DEREF) {
        if (!unparse_child(unpar, operand, prec, false)) return false;

        switch (op) {
        case EL_SEMA_UNARY_OP_POST_INC: return el_unparser_push_punct(unpar, EL_TT_INC);
        case EL_SEMA_UNARY_OP_POST_DEC: return el_unparser_push_punct(unpar, EL_TT_DEC);
        case EL_SEMA_UNARY_OP_DEREF:    return el_unparser_push_punct(unpar, EL_TT_CARET);
        case EL_SEMA_UNARY_OP_OPT_UNWRAP: return el_unparser_push_punct(unpar, EL_TT_LOGICAL_NOT);
        default: EL_UNREACHABLE("not a postfix unary");
        }
    }

    switch (op) {
    case EL_SEMA_UNARY_OP_POS:     if (!el_unparser_push_punct(unpar, EL_TT_PLUS))        return false; break;
    case EL_SEMA_UNARY_OP_NEG:     if (!el_unparser_push_punct(unpar, EL_TT_MINUS))       return false; break;
    case EL_SEMA_UNARY_OP_NOT:     if (!el_unparser_push_punct(unpar, EL_TT_LOGICAL_NOT)) return false; break;
    case EL_SEMA_UNARY_OP_BW_NOT:  if (!el_unparser_push_punct(unpar, EL_TT_BITWISE_NOT)) return false; break;
    case EL_SEMA_UNARY_OP_ADDROF:  if (!el_unparser_push_punct(unpar, EL_TT_BITWISE_AND)) return false; break;
    case EL_SEMA_UNARY_OP_PRE_INC: if (!el_unparser_push_punct(unpar, EL_TT_INC))         return false; break;
    case EL_SEMA_UNARY_OP_PRE_DEC: if (!el_unparser_push_punct(unpar, EL_TT_DEC))         return false; break;
    default: EL_UNREACHABLE("not a prefix unary");
    }

    return unparse_child(unpar, operand, prec, true);
}

static bool unparse_binary(ElUnparser* unpar, ElAstExpr* expr) {
    ElAstBinOp op = expr->as.binary.op;
    int prec = bin_op_prec(op);

    if (op == EL_SEMA_BIN_OP_INDEX) {
        if (!unparse_child(unpar, expr->as.binary.left, prec, false)) return false;
        if (!el_unparser_push_punct(unpar, EL_TT_LBRACKET))           return false;
        if (!el_unparser_unparse_expr(unpar, expr->as.binary.right))  return false;
        return el_unparser_push_punct(unpar, EL_TT_RBRACKET);
    }

    if (!unparse_child(unpar, expr->as.binary.left, prec, false)) return false;
    if (!el_unparser_push_punct(unpar, bin_op_token(op)))         return false;
    return unparse_child(unpar, expr->as.binary.right, prec, true);
}

static bool unparse_call(ElUnparser* unpar, ElAstExpr* expr) {
    int prec = PREC_POSTFIX;
    if (!unparse_child(unpar, expr->as.call.callee, prec, false)) return false;
    if (!el_unparser_push_punct(unpar, EL_TT_LPAREN))             return false;

    for (ElAstToI* arg = expr->as.call.args; arg != NULL; arg = arg->next) {
        if (!el_unparser_unparse_toi(unpar, arg)) return false;
        if (arg->next != NULL) {
            if (!el_unparser_push_punct(unpar, EL_TT_COMMA)) return false;
        }
    }

    return el_unparser_push_punct(unpar, EL_TT_RPAREN);
}

static bool unparse_cast(ElUnparser* unpar, ElAstExpr* expr) {
    int prec = PREC_CAST;
    if (!unparse_child(unpar, expr->as.cast.expr, prec, false)) return false;
    if (!el_unparser_push_kw(unpar, EL_TT_KW_AS)) return false;
    return el_unparser_unparse_type(unpar, expr->as.cast.type);
}

static bool unparse_member(ElUnparser* unpar, ElAstExpr* expr) {
    int prec = PREC_POSTFIX;
    if (!unparse_child(unpar, expr->as.member.expr, prec, false)) return false;
    if (!el_unparser_push_punct(unpar, EL_TT_DOT)) return false;
    return el_unparser_push_ident(unpar, expr->as.member.name);
}

static bool unparse_tmember(ElUnparser* unpar, ElAstExpr* expr) {
    int prec = PREC_POSTFIX;
    if (!unparse_child(unpar, expr->as.tmember.expr, prec, false)) return false;
    if (!el_unparser_push_punct(unpar, EL_TT_DOT)) return false;

    return el_unparser_push_fmt(
        unpar, EL_TT_INT_LITERAL, "%zu", expr->as.tmember.index
    );
}

static bool unparse_typedinit(ElUnparser* unpar, ElAstExpr* expr) {
    if (expr->as.typedinit.scls == EL_STORAGECLS_STATIC) {
        if (!el_unparser_push_kw(unpar, EL_TT_KW_STATIC)) return false;
    }
    if (!el_unparser_unparse_type(unpar, expr->as.typedinit.type)) return false;
    return el_unparser_unparse_init(unpar, expr->as.typedinit.init);
}

bool el_unparser_unparse_expr(ElUnparser* unpar, ElAstExpr* expr) {
    switch (expr->type) {
    case EL_AST_EXPR_LITERAL:   return unparse_literal(unpar, expr);
    case EL_AST_EXPR_IDENT:     return _el_unparser_unparse_ident(unpar, &expr->as.ident);
    case EL_AST_EXPR_UNARY:     return unparse_unary(unpar, expr);
    case EL_AST_EXPR_BINARY:    return unparse_binary(unpar, expr);
    case EL_AST_EXPR_CALL:      return unparse_call(unpar, expr);
    case EL_AST_EXPR_CAST:      return unparse_cast(unpar, expr);
    case EL_AST_EXPR_MEMBER:    return unparse_member(unpar, expr);
    case EL_AST_EXPR_TMEMBER:   return unparse_tmember(unpar, expr);
    case EL_AST_EXPR_TYPEDINIT: return unparse_typedinit(unpar, expr);
    }
    EL_UNREACHABLE_ENUM_VAL(ElAstExprType, expr->type);
}
