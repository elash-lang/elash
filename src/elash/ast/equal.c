#include <elash/ast/equal.h>
#include <elash/util/assert.h>

bool el_ast_equal_designator(const ElAstDesignator* a, const ElAstDesignator* b);

bool el_ast_equal_type(const ElAstType* a, const ElAstType* b) {
    if (a == b) return true;
    if (!a || !b) return false;
    if (a->kind != b->kind) return false;
    if (a->mut != b->mut) return false;

    switch (a->kind) {
    case EL_AST_TYPE_STRUCT: {
        if (a->as.struct_.count != b->as.struct_.count) return false;
        ElAstDecl* fa = a->as.struct_.fields;
        ElAstDecl* fb = b->as.struct_.fields;
        while (fa != NULL && fb != NULL) {
            if (!el_ast_equal_decl(fa, fb)) return false;
            fa = fa->next;
            fb = fb->next;
        }
        return fa == fb;
    }
    case EL_AST_TYPE_TUPLE: {
        if (a->as.tuple.count != b->as.tuple.count) return false;
        ElAstType* ta = a->as.tuple.head;
        ElAstType* tb = b->as.tuple.head;
        while (ta != NULL && tb != NULL) {
            if (!el_ast_equal_type(ta, tb)) return false;
            ta = ta->next;
            tb = tb->next;
        }
        return ta == tb;
    }
    case EL_AST_TYPE_NAME:
        return el_sv_eql(a->as.name->name, b->as.name->name);
    case EL_AST_TYPE_REF:
        return el_ast_equal_type(a->as.ref.base, b->as.ref.base);
    case EL_AST_TYPE_OPT:
        return el_ast_equal_type(a->as.opt.base, b->as.opt.base);
    case EL_AST_TYPE_ARRAY:
        return el_ast_equal_type(a->as.array.base, b->as.array.base) &&
               el_ast_equal_expr(a->as.array.size, b->as.array.size);
    case EL_AST_TYPE_SLICE:
        return el_ast_equal_type(a->as.slice.base, b->as.slice.base) &&
               a->as.slice.is_raw == b->as.slice.is_raw;
    }
    EL_UNREACHABLE_ENUM_VAL(ElAstTypeKind, a->kind);
}

bool el_ast_equal_expr(const ElAstExpr* a, const ElAstExpr* b) {
    if (a == b) return true;
    if (!a || !b) return false;
    if (a->type != b->type) return false;

    switch (a->type) {
    case EL_AST_EXPR_BINARY:
        return el_ast_equal_expr(a->as.binary.left, b->as.binary.left) &&
               a->as.binary.op == b->as.binary.op &&
               el_ast_equal_expr(a->as.binary.right, b->as.binary.right);
    case EL_AST_EXPR_UNARY:
        return a->as.unary.op == b->as.unary.op &&
               el_ast_equal_expr(a->as.unary.operand, b->as.unary.operand);
    case EL_AST_EXPR_LITERAL:
        if (a->as.literal.kind != b->as.literal.kind) return false;
        switch (a->as.literal.kind) {
        case EL_AST_LIT_INT: return el_i128_eq(a->as.literal.of.int_, b->as.literal.of.int_);
        case EL_AST_LIT_FLOAT: return a->as.literal.of.float_ == b->as.literal.of.float_;
        case EL_AST_LIT_CHAR: return a->as.literal.of.char_ == b->as.literal.of.char_;
        case EL_AST_LIT_STRING: return el_sv_eql(a->as.literal.of.str_, b->as.literal.of.str_);
        case EL_AST_LIT_BOOL: return a->as.literal.of.bool_ == b->as.literal.of.bool_;
        case EL_AST_LIT_NULL: return true;
        }
        EL_UNREACHABLE_ENUM_VAL(ElAstLiteralKind, a->as.literal.kind);
    case EL_AST_EXPR_TYPEDINIT:
        return a->as.typedinit.scls == b->as.typedinit.scls &&
               el_ast_equal_type(a->as.typedinit.type, b->as.typedinit.type) &&
               el_ast_equal_init(a->as.typedinit.init, b->as.typedinit.init);
    case EL_AST_EXPR_IDENT:
        return el_sv_eql(a->as.ident.name, b->as.ident.name);
    case EL_AST_EXPR_CALL: {
        if (a->as.call.arg_count != b->as.call.arg_count) return false;
        if (!el_ast_equal_expr(a->as.call.callee, b->as.call.callee)) return false;

        ElAstToI* aa = a->as.call.args;
        ElAstToI* ab = b->as.call.args;
        while (aa != NULL && ab != NULL) {
            if (!el_ast_equal_toi(aa, ab)) return false;
            aa = aa->next;
            ab = ab->next;
        }
        return aa == ab;
    }
    case EL_AST_EXPR_CAST:
        return el_ast_equal_expr(a->as.cast.expr, b->as.cast.expr) &&
               el_ast_equal_type(a->as.cast.type, b->as.cast.type);
    case EL_AST_EXPR_MEMBER:
        return el_ast_equal_expr(a->as.member.expr, b->as.member.expr) &&
               el_sv_eql(a->as.member.name, b->as.member.name);
    case EL_AST_EXPR_TMEMBER:
        return el_ast_equal_expr(a->as.tmember.expr, b->as.tmember.expr) &&
               a->as.tmember.index == b->as.tmember.index;
    }
    EL_UNREACHABLE_ENUM_VAL(ElAstExprType, a->type);
}

bool el_ast_equal_func_param(const ElAstFuncParam* a, const ElAstFuncParam* b) {
    if (a == b) return true;
    if (!a || !b) return false;
    return el_ast_equal_type(a->type, b->type) && el_sv_eql(a->name->name, b->name->name);
}

bool el_ast_equal_func_signature(const ElAstFuncSignature* a, const ElAstFuncSignature* b) {
    if (!el_ast_equal_type(a->ret_type, b->ret_type)) return false;
    if (!el_sv_eql(a->name->name, b->name->name)) return false;
    if (a->params.count != b->params.count) return false;
    ElAstFuncParam* pa = a->params.head;
    ElAstFuncParam* pb = b->params.head;
    while (pa != NULL && pb != NULL) {
        if (!el_ast_equal_func_param(pa, pb)) return false;
        pa = pa->next;
        pb = pb->next;
    }
    return pa == pb;
}

bool el_ast_equal_declarator(const ElAstDeclarator* a, const ElAstDeclarator* b) {
    return el_sv_eql(a->name->name, b->name->name) &&
           el_ast_equal_init(a->init, b->init);
}

bool el_ast_equal_declarators(const ElAstDeclarator* a, const ElAstDeclarator* b) {
    for (; a != NULL && b != NULL; a = a->next, b = b->next) {
        if (!el_ast_equal_declarator(a, b)) return false;
    }
    return a == NULL && b == NULL;
}

bool el_ast_equal_decl(const ElAstDecl* a, const ElAstDecl* b) {
    if (a == b) return true;
    if (!a || !b) return false;
    if (a->type != b->type) return false;

    switch (a->type) {
    case EL_AST_DECL_ALIAS:
        return el_sv_eql(a->as.alias.name, b->as.alias.name) &&
               el_ast_equal_toe(&a->as.alias.target, &b->as.alias.target);
    case EL_AST_DECL_TYPEDEF:
        return el_sv_eql(a->as.typedef_.name, b->as.typedef_.name) &&
               el_ast_equal_type(a->as.typedef_.target, b->as.typedef_.target);
    case EL_AST_DECL_VAR_DEF:
        if (a->as.var_def.is_static != b->as.var_def.is_static)
            return false;
        if (!el_ast_equal_type(a->as.var_def.type, b->as.var_def.type))
            return false;
        return el_ast_equal_declarators(a->as.var_def.declarators, b->as.var_def.declarators);
    case EL_AST_DECL_VAR_DECL:
        if (!el_ast_equal_type(a->as.var_decl.type, b->as.var_decl.type))
            return false;
        return el_ast_equal_declarators(a->as.var_decl.declarators, b->as.var_decl.declarators);
    case EL_AST_DECL_FUNC_DEF: {
        if (!el_ast_equal_func_signature(&a->as.func_def.sig, &b->as.func_def.sig)) return false;
        ElAstStmt* sa = a->as.func_def.block->stmts;
        ElAstStmt* sb = b->as.func_def.block->stmts;
        while (sa != NULL && sb != NULL) {
            if (!el_ast_equal_stmt(sa, sb)) return false;
            sa = sa->next;
            sb = sb->next;
        }
        return sa == sb;
    }
    case EL_AST_DECL_FUNC_DECL:
        return el_ast_equal_func_signature(&a->as.func_decl.sig, &b->as.func_decl.sig);
    }
    EL_UNREACHABLE_ENUM_VAL(ElAstDeclType, a->type);
}

bool el_ast_equal_stmt(const ElAstStmt* a, const ElAstStmt* b) {
    if (a == b) return true;
    if (!a || !b) return false;
    if (a->type != b->type) return false;

    switch (a->type) {
    case EL_AST_STMT_EXPR:
        return el_ast_equal_expr(a->as.expr, b->as.expr);
    case EL_AST_STMT_RETURN:
        return el_ast_equal_init(a->as.return_.value, b->as.return_.value);
    case EL_AST_STMT_DECL:
        return el_ast_equal_decl(a->as.decl, b->as.decl);
    case EL_AST_STMT_ASSIGN:
        return el_ast_equal_expr(a->as.assign.target, b->as.assign.target) &&
               el_ast_equal_init(a->as.assign.value, b->as.assign.value);
    case EL_AST_STMT_BLOCK: {
        ElAstStmt* sa = a->as.block.stmts;
        ElAstStmt* sb = b->as.block.stmts;
        while (sa != NULL && sb != NULL) {
            if (!el_ast_equal_stmt(sa, sb)) return false;
            sa = sa->next;
            sb = sb->next;
        }
        return sa == sb;
    }
    case EL_AST_STMT_CASSIGN:
        return a->as.cassign.op == b->as.cassign.op &&
               el_ast_equal_expr(a->as.cassign.target, b->as.cassign.target) &&
               el_ast_equal_init(a->as.cassign.value, b->as.cassign.value);
    case EL_AST_STMT_IF:
        return el_ast_equal_expr(a->as.if_.cond, b->as.if_.cond) &&
               el_ast_equal_stmt(a->as.if_.then, b->as.if_.then) &&
               el_ast_equal_stmt(a->as.if_.else_, b->as.if_.else_);
    case EL_AST_STMT_WHILE:
        return el_ast_equal_expr(a->as.while_.cond, b->as.while_.cond) &&
               el_ast_equal_stmt(a->as.while_.body, b->as.while_.body);
    case EL_AST_STMT_BREAK:
    case EL_AST_STMT_CONTINUE:
        return true;
    }
    EL_UNREACHABLE_ENUM_VAL(ElAstStmtType, a->type);
}

bool el_ast_equal_designator(const ElAstDesignator* a, const ElAstDesignator* b) {
    if (a == b) return true;
    if (!a || !b) return false;
    if (a->kind != b->kind) return false;
    switch (a->kind) {
    case EL_AST_DESIGNATOR_MEMBER:
        return el_sv_eql(a->as.member, b->as.member);
    case EL_AST_DESIGNATOR_TMEMBER:
        return a->as.tmember == b->as.tmember;
    case EL_AST_DESIGNATOR_INDEX:
        return el_ast_equal_expr(a->as.index, b->as.index);
    }
    EL_UNREACHABLE_ENUM_VAL(ElAstDesignatorKind, a->kind);
}

static bool equal_desig_element(ElAstDesigInitElem* ea, ElAstDesigInitElem* eb) {
    if (ea->desig_count != eb->desig_count) return false;

    ElAstDesignator* da = ea->head;
    ElAstDesignator* db = eb->head;
    while (da != NULL && db != NULL) {
        if (!el_ast_equal_designator(da, db)) return false;
        da = da->next;
        db = db->next;
    }

    if (da != db) return false;
    return el_ast_equal_init(ea->init, eb->init);
}

bool el_ast_equal_init(const ElAstInit* a, const ElAstInit* b) {
    if (a == b) return true;
    if (!a || !b) return false;
    if (a->kind != b->kind) return false;

    switch (a->kind) {
    case EL_AST_INIT_EXPR:
        return el_ast_equal_expr(a->expr, b->expr);
    case EL_AST_INIT_LIST:
        if (a->list.count != b->list.count) return false;
        ElAstInit* ia = a->list.head;
        ElAstInit* ib = b->list.head;
        while (ia != NULL && ib != NULL) {
            if (!el_ast_equal_init(ia, ib)) return false;
            ia = ia->next;
            ib = ib->next;
        }
        return ia == ib;
    case EL_AST_INIT_DESIG: {
        if (a->desig.count != b->desig.count) return false;
        ElAstDesigInitElem* ea = a->desig.head;
        ElAstDesigInitElem* eb = b->desig.head;
        while (ea != NULL && eb != NULL) {
            if (!equal_desig_element(ea, eb)) return false;
            ea = ea->next;
            eb = eb->next;
        }
        return ea == eb;
    }
    case EL_AST_INIT_EMPTY:
        return true;
    }
    EL_UNREACHABLE_ENUM_VAL(ElAstInitKind, a->kind);
}

bool el_ast_equal_module(const ElAstModule* a, const ElAstModule* b) {
    if (a == b) return true;
    if (!a || !b) return false;
    if (a->count != b->count) return false;

    ElAstDecl* da = a->head;
    ElAstDecl* db = b->head;
    while (da != NULL && db != NULL) {
        if (!el_ast_equal_decl(da, db)) return false;
        da = da->next;
        db = db->next;
    }

    return da == db;
}

bool el_ast_equal_unr(const ElAstUnr* a, const ElAstUnr* b) {
    if (a == b) return true;
    if (!a || !b) return false;
    if (a->kind != b->kind) return false;
    switch (a->kind) {
    case EL_AST_UNR_IDENT:
        return el_sv_eql(a->as.ident->name, b->as.ident->name);
    case EL_AST_UNR_INDEX:
        return el_ast_equal_unr(a->as.index.base, b->as.index.base) &&
               el_ast_equal_unr(a->as.index.index, b->as.index.index) &&
               el_ast_equal_expr(a->as.index.index_expr, b->as.index.index_expr);
    }
    EL_UNREACHABLE_ENUM_VAL(ElAstUnrKind, a->kind);
}

bool el_ast_equal_toe(const ElAstToE* a, const ElAstToE* b) {
    if (a == b) return true;
    if (!a || !b) return false;
    if (a->kind != b->kind) return false;
    switch (a->kind) {
    case EL_AST_TOE_TYPE:
        return el_ast_equal_type(a->as.type, b->as.type);
    case EL_AST_TOE_EXPR:
        return el_ast_equal_expr(a->as.expr, b->as.expr);
    case EL_AST_TOE_UNR:
        return el_ast_equal_unr(a->as.unr, b->as.unr);
    }
    EL_UNREACHABLE_ENUM_VAL(ElAstToEKind, a->kind);
}

bool el_ast_equal_toi(const ElAstToI* a, const ElAstToI* b) {
    if (a == b) return true;
    if (!a || !b) return false;
    if (a->kind != b->kind) return false;
    switch (a->kind) {
    case EL_AST_TOI_TYPE:
        return el_ast_equal_type(a->as.type, b->as.type);
    case EL_AST_TOI_INIT:
        return el_ast_equal_init(a->as.init, b->as.init);
    case EL_AST_TOI_UNR:
        return el_ast_equal_unr(a->as.unr, b->as.unr);
    }
    EL_UNREACHABLE_ENUM_VAL(ElAstToIKind, a->kind);
}
