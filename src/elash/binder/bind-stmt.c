#include "binder-internals.h"

#include <elash/diag/engine.h>
#include <elash/util/assert.h>
#include <elash/util/todo.h>

#include <elash/hir/tree/stmt/block.h>
#include <elash/hir/tree/stmt/return.h>

bool _el_binder_is_lvalue(ElHirExpr* expr) {
    if (expr == NULL) return false;
    return (expr->kind == EL_HIR_EXPR_UNARY && expr->as.unary.op == EL_UNARY_OP_DEREF)
        || (expr->kind == EL_HIR_EXPR_UNARY && expr->as.unary.op == EL_UNARY_OP_OPT_UNWRAP)
        || (expr->kind == EL_HIR_EXPR_BINARY && expr->as.binary.op == EL_BIN_OP_INDEX)
        || (expr->kind == EL_HIR_EXPR_SYMBOL)
        || (expr->kind == EL_HIR_EXPR_MEMBER)
        || (expr->kind == EL_HIR_EXPR_TMEMBER);
}

ElHirBlockStmt _el_bind_block(ElBinder* binder, ElAstBlockStmt* in) {
    ElHirStmt* head = NULL;
    ElHirStmt* tail = NULL;

    for (ElAstStmt* curr = in->stmts; curr != NULL; curr = curr->next) {
        ElHirStmt* binded = el_bind_stmt(binder, curr);
        if (binded != NULL) {
            el_hir_stmt_list_append(&head, &tail, binded);
        }
    }

    return (ElHirBlockStmt) { .stmts = head };
}

static ElHirStmt* bind_return(ElBinder* binder, ElAstStmt* in) {
    ElHirExpr* val = NULL;
    if (in->as.return_.value != NULL) {
        val = el_bind_init(
            binder, in->as.return_.value,
            binder->current_func->as.func.type->as.func.ret_type,
            EL_STORAGECLS_LOCAL
        );
        if (val == NULL) goto skip;
    }

    bool is_void_func
        = el_hir_type_eql(binder->current_func->as.func.type->as.func.ret_type, binder->builtins->type_void);

    if (val == NULL) {
        if (!is_void_func) {
            el_diag_report(
                binder->diag, EL_DIAG_ERROR, "sema.return-val-expected",
                in->span, "expected return value in non-void function",
            );
        }
    } else {
        bool is_val_void = el_hir_type_eql(val->type, binder->builtins->type_void);
        if (is_void_func) {
            if (!is_val_void) {
                el_diag_report(
                    binder->diag, EL_DIAG_ERROR, "sema.return-val-in-void-func",
                    in->span, "void function should not return a value",
                );
            }
        } else {
            if (is_val_void) {
                el_diag_report(
                    binder->diag, EL_DIAG_ERROR, "sema.returning-void",
                    in->span, "cannot return void value from non-void function",
                );
            } else {
                val = _el_binder_implicit_cast(binder, in->span, val, binder->current_func->as.func.type->as.func.ret_type);
                if (val == NULL) return NULL;
            }
        }
    }

skip:
    return el_hir_new_return_stmt(binder->arena, in->span, val);
}

#define REPORT_ASSIGN_TO_RVALUE(BINDER, SPAN)                \
    el_diag_report(                                          \
        (BINDER)->diag, EL_DIAG_ERROR, "sema.assign-rvalue", \
        (SPAN), "cannot assign to rvalue",                   \
    )                                                        \

static ElHirStmt* bind_assign(ElBinder* binder, ElAstStmt* in, ElAstAssignStmt* assign) {
    ElHirExpr* target = el_bind_expr(binder, assign->target);
    if (!_el_binder_is_lvalue(target)) {
        REPORT_ASSIGN_TO_RVALUE(binder, in->span);
        return NULL;
    }
    if (!_el_binder_ensure_writable(binder, assign->target->span, target))
        return NULL;

    ElHirExpr* value = el_bind_init(binder, assign->value, target->type, EL_STORAGECLS_LOCAL);
    if (value == NULL) return NULL;

    return el_hir_new_assign_stmt(
        binder->arena, in->span,
        target, value
    );
}

static ElHirStmt* bind_compound_assign(ElBinder* binder, ElAstStmt* in, ElAstCompoundAssignStmt* cassign) {
    ElHirExpr* target = el_bind_expr(binder, cassign->target);
    if (!_el_binder_is_lvalue(target)) {
        REPORT_ASSIGN_TO_RVALUE(binder, in->span);
        return NULL;
    }
    if (!_el_binder_ensure_writable(binder, cassign->target->span, target))
        return NULL;

    ElHirExpr* value = el_bind_init(binder, cassign->value, target->type, EL_STORAGECLS_LOCAL);
    if (value == NULL) return NULL;

    return el_hir_new_compound_assign_stmt(
        binder->arena, in->span,
        cassign->op, target, value
    );
}

static ElHirStmt* bind_if(ElBinder* binder, ElAstStmt* in, ElAstIfStmt* if_) {
    bool has_init = if_->init != NULL;
    if (has_init) _el_binder_push_scope(binder);

    ElHirStmt* init_stmt = NULL;
    if (has_init) {
        init_stmt = el_bind_stmt(binder, if_->init);
        if (init_stmt == NULL) goto e1;
    }

    ElHirExpr* cond = el_bind_expr(binder, if_->cond);
    if (cond == NULL) goto e1;

    cond = _el_binder_implicit_cast(binder, if_->cond->span, cond, binder->builtins->type_bool);
    if (cond == NULL) goto e1;

    ElHirStmt* then = el_bind_stmt(binder, if_->then);
    if (then == NULL) goto e1;

    ElHirStmt* else_ = NULL;
    if (if_->else_ != NULL) {
        else_ = el_bind_stmt(binder, if_->else_);
        if (else_ == NULL) goto e1;
    }

    if (has_init) _el_binder_pop_scope(binder);

    ElHirStmt* if_stmt = el_hir_new_if_stmt(
        binder->arena, in->span,
        cond, then, else_
    );

    if (!has_init) return if_stmt;

    // we perform desugaring directly here so we don't need to add any
    // additional logic to the lowerer and new fields to the hir structures
    ElHirStmt *head = NULL, *tail = NULL;
    el_hir_stmt_list_append(&head, &tail, init_stmt);
    el_hir_stmt_list_append(&head, &tail, if_stmt);
    return el_hir_new_block_stmt(binder->arena, in->span, head);

e1:
    if (has_init)
        _el_binder_pop_scope(binder);
    return NULL;
}

static ElHirStmt* bind_while(ElBinder* binder, ElAstStmt* in, ElAstWhileStmt* while_) {
    bool has_init = while_->init != NULL;
    if (has_init) _el_binder_push_scope(binder);

    ElHirStmt* init_stmt = NULL;
    if (has_init) {
        init_stmt = el_bind_stmt(binder, while_->init);
        if (init_stmt == NULL) goto e1;
    }

    binder->loop_depth++;
    ElHirExpr* cond = el_bind_expr(binder, while_->cond);
    if (cond == NULL) goto e2;

    cond = _el_binder_implicit_cast(binder, while_->cond->span, cond, binder->builtins->type_bool);
    if (cond == NULL) goto e2;

    ElHirStmt* body = el_bind_stmt(binder, while_->body);
    if (body == NULL) goto e2;

    binder->loop_depth--;
    if (has_init) _el_binder_pop_scope(binder);

    ElHirStmt* while_stmt = el_hir_new_while_stmt(binder->arena, in->span, cond, body);
    if (!has_init) return while_stmt;

    ElHirStmt *head = NULL, *tail = NULL;
    el_hir_stmt_list_append(&head, &tail, init_stmt);
    el_hir_stmt_list_append(&head, &tail, while_stmt);
    return el_hir_new_block_stmt(binder->arena, in->span, head);

e2:
    binder->loop_depth--;
e1:
    if (has_init) _el_binder_pop_scope(binder);
    return NULL;
}

static ElHirStmt* bind_expr_stmt(ElBinder* binder, ElAstStmt* in) {
    ElHirExpr* expr = el_bind_expr(binder, in->as.expr);
    if (expr == NULL) return NULL;

    expr = _el_binder_apply_default_type(binder, expr);
    if (expr == NULL) return NULL;

    switch (_el_binder_redundancy_if_ignored(binder, expr)) {
    case REDUNDANCY_NONE:
        break;
    case REDUNDANCY_FULL:
        el_diag_report(
            binder->diag, EL_DIAG_WARN, "sema.side-effects",
            in->span, "statement has no side effects",
        );
        el_diag_help(
            binder->diag, "the entire statement can be safely removed"
        );
        return NULL;
    case REDUNDANCY_PARTIAL:
        el_diag_report(
            binder->diag, EL_DIAG_WARN, "sema.ignored",
            in->span, "result of operation is unused",
        );
        if (expr->kind == EL_HIR_EXPR_UNARY && el_unary_op_is_incdec(expr->as.unary.op)) {
            el_diag_help(
                binder->diag, "use prefix ${op} when the value is unused",
                EL_DIAG_STRING("op", expr->as.unary.op == EL_UNARY_OP_POST_INC ? EL_SV("incrementation") : EL_SV("decrementation"))
            );
        }
        break;
    }

    return el_hir_new_expr_stmt(binder->arena, in->span, expr);
}

static ElHirStmt* _bind_stmt_internal(ElBinder* binder, ElAstStmt* in) {
    switch (in->type) {
    case EL_AST_STMT_BLOCK: {
        _el_binder_push_scope(binder);
        ElHirBlockStmt block = _el_bind_block(binder, &in->as.block);
        _el_binder_pop_scope(binder);
        return el_hir_new_block_stmt(binder->arena, in->span, block.stmts);
    }
    case EL_AST_STMT_DECL: {
        ElHirDecl* decl = el_bind_decl(binder, in->as.decl);
        if (decl == NULL) return NULL;
        return el_hir_new_decl_stmt(binder->arena, in->span, decl);
    }

    case EL_AST_STMT_IF:
        return bind_if(binder, in, &in->as.if_);
    case EL_AST_STMT_WHILE:
        return bind_while(binder, in, &in->as.while_);

    case EL_AST_STMT_RETURN:
        return bind_return(binder, in);
    case EL_AST_STMT_EXPR:
        return bind_expr_stmt(binder, in);

    case EL_AST_STMT_BREAK:
        if (binder->loop_depth <= 0)
            return el_diag_report(
                binder->diag, EL_DIAG_ERROR, "sema.break-outside-loop",
                in->span, "'break' can only be used inside loops",
            );
        return el_hir_new_break_stmt(binder->arena, in->span);
    case EL_AST_STMT_CONTINUE:
        if (binder->loop_depth <= 0)
            return el_diag_report(
                binder->diag, EL_DIAG_ERROR, "sema.continue-outside-loop",
                in->span, "'continue' can only be used inside loops",
            );
        return el_hir_new_continue_stmt(binder->arena, in->span);

    case EL_AST_STMT_ASSIGN:
        return bind_assign(binder, in, &in->as.assign);
    case EL_AST_STMT_CASSIGN:
        return bind_compound_assign(binder, in, &in->as.cassign);
    }
    EL_UNREACHABLE_ENUM_VAL(ElAstStmtType, in->type);
}

ElHirStmt* el_bind_stmt(ElBinder* binder, ElAstStmt* in) {
    EL_ASSERT(in != NULL, "should not be NULL");
    el_prof_begin_sub(binder->prof, binder->pss_stmt);
    ElHirStmt* result = _bind_stmt_internal(binder, in);
    el_prof_finish_sub(binder->prof, binder->pss_stmt);
    return result;
}
