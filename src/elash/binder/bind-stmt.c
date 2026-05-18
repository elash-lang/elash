#include <elash/binder/binder.h>
#include <elash/util/assert.h>
#include <elash/util/todo.h>

#include <elash/diag/engine.h>
#include <elash/diag/meta.h>

#include <elash/hir/tree/stmt/block.h>
#include <elash/hir/tree/stmt/return.h>

bool _el_binder_is_lvalue(ElHirExpr* expr) {
    // TODO: simplified
    return (expr->kind == EL_HIR_EXPR_UNARY && expr->as.unary.op == EL_SEMA_UNARY_OP_DEREF)
        || (expr->kind == EL_HIR_EXPR_BINARY && expr->as.binary.op == EL_SEMA_BIN_OP_INDEX)
        || (expr->kind == EL_HIR_EXPR_SYMBOL);
}

ElHirBlockStmt _el_binder_bind_block(ElBinder* binder, ElAstBlockStmt* in) {
    ElHirStmt* head = NULL;
    ElHirStmt* tail = NULL;

    for (ElAstStmt* curr = in->stmts; curr != NULL; curr = curr->next) {
        ElHirStmt* binded = el_binder_bind_stmt(binder, curr);
        if (binded) {
            el_hir_stmt_list_append(&head, &tail, binded);
        }
    }

    return (ElHirBlockStmt) { .stmts = head };
}

ElHirStmt* _el_binder_bind_return(ElBinder* binder, ElAstStmt* in) {
    ElHirExpr* val = el_binder_bind_expr(binder, in->as.return_.value);
    bool is_void_func = el_sema_type_eql(binder->current_func->as.func.ret_type, binder->builtins->type_void);

    if (val == NULL) {
        if (!is_void_func) {
            el_diag_report(
                binder->diag, EL_DIAG_ERROR, "sema.return-val-expected",
                in->span,
                "expected return value in non-void function"
            );
        }
    } else {
        bool is_val_void = el_sema_type_eql(val->type, binder->builtins->type_void);
        if (is_void_func) {
            if (!is_val_void) {
                el_diag_report(
                    binder->diag, EL_DIAG_ERROR, "sema.return-val-in-void-func",
                    in->span,
                    "void function should not return a value"
                );
            }
        } else {
            if (is_val_void) {
                 el_diag_report(
                    binder->diag, EL_DIAG_ERROR, "sema.returning-void",
                    in->span,
                    "cannot return void value from non-void ction"
                );
            } else if (!el_sema_type_eql(val->type, binder->current_func->as.func.ret_type)) {
                el_diag_report(
                    binder->diag, EL_DIAG_ERROR, "sema.type-mismatch",
                    in->span,
                    "incompatible return type"
                );
            }
        }
    }

    return el_hir_new_return_stmt(binder->hir_arena, val);
}

ElHirStmt* _el_binder_bind_var_definition(ElBinder* binder, ElAstStmt* in) {
    ElType* type = _el_binder_bind_type(binder, in->as.var_def.type);
    if (!type) return NULL;

    if (type->kind == EL_TYPE_PRIM && type->as.prim.kind == EL_PRIMTYPE_VOID) {
        el_diag_report(
            binder->diag, EL_DIAG_ERROR, "sema.incomplete-type",
            in->as.var_def.type->span,
            "cannot declare variable of incomplete type 'void'"
        );
        return NULL;
    }

    ElSymbol* sym = el_sema_new_var_symbol(binder->sym_arena, binder->sym_id_counter++, in->as.var_def.name->name, type);
    if (!el_sema_scope_insert(binder->current_scope, sym)) {
        el_diag_report(
            binder->diag, EL_DIAG_ERROR, "sema.redeclaration",
            in->as.var_def.name->span,
            "redeclaration of symbol '${name}'",
            EL_DIAG_STRING("name", sym->name)
        );
        return NULL;
    }

    ElHirExpr* init = NULL;
    if (in->as.var_def.init) {
        init = el_binder_bind_init(binder, in->as.var_def.init, type);
        if (!init) return NULL;
    }

    return el_hir_new_var_def_stmt(binder->hir_arena, sym, init);
}

#define REPORT_ASSIGN_TO_RVALUE(BINDER, SPAN)                \
    el_diag_report(                                          \
        (BINDER)->diag, EL_DIAG_ERROR, "sema.assign-rvalue", \
        (SPAN), "cannot assign to rvalue",                   \
    )                                                        \

ElHirStmt* _el_binder_bind_assign(ElBinder* binder, ElAstStmt* in, ElAstAssignStmt* assign) {
    ElHirExpr* target = el_binder_bind_expr(binder, assign->target);
    if (!_el_binder_is_lvalue(target)) {
        REPORT_ASSIGN_TO_RVALUE(binder, in->span);
        return NULL;
    }

    return el_hir_new_assign_stmt(
        binder->hir_arena,
        target,
        el_binder_bind_expr(binder, assign->value)
    );
}

ElHirStmt* _el_binder_bind_compound_assign(ElBinder* binder, ElAstStmt* in, ElAstCompoundAssignStmt* cassign) {
    ElHirExpr* target = el_binder_bind_expr(binder, cassign->target);
    if (!_el_binder_is_lvalue(target)) {
        REPORT_ASSIGN_TO_RVALUE(binder, in->span);
        return NULL;
    }

    return el_hir_new_compound_assign_stmt(
        binder->hir_arena,
        cassign->op, target,
        el_binder_bind_expr(binder, cassign->value)
    );
}

ElHirStmt* el_binder_bind_stmt(ElBinder* binder, ElAstStmt* in) {
    switch (in->type) {
    case EL_AST_STMT_BLOCK: {
        ElHirBlockStmt block = _el_binder_bind_block(binder, &in->as.block);
        return el_hir_new_block_stmt(binder->hir_arena, block.stmts);
    }
    case EL_AST_STMT_RETURN:
        return _el_binder_bind_return(binder, in);
    case EL_AST_STMT_EXPR: {
        ElHirExpr* expr = el_binder_bind_expr(binder, in->as.expr);
        return el_hir_new_expr_stmt(binder->hir_arena, expr);
    }

    case EL_AST_STMT_IF:
        return el_hir_new_if_stmt(
            binder->hir_arena,
            el_binder_bind_expr(binder, in->as.if_.cond),
            el_binder_bind_stmt(binder, in->as.if_.then),
            in->as.if_.else_ != NULL
                ? el_binder_bind_stmt(binder, in->as.if_.else_)
                : NULL
        );
    case EL_AST_STMT_WHILE:
        binder->loop_depth++;
        ElHirStmt* body = el_binder_bind_stmt(binder, in->as.while_.body);
        binder->loop_depth--;

        return el_hir_new_while_stmt(
            binder->hir_arena,
            el_binder_bind_expr(binder, in->as.while_.cond),
            body
        );

    case EL_AST_STMT_BREAK:
        if (binder->loop_depth <= 0) {
            el_diag_report(
                binder->diag, EL_DIAG_ERROR, "sema.break-outside-loop",
                in->span, "'break' can only be used inside loops.",
            );
            return NULL;
        }
        return el_hir_new_break_stmt(binder->hir_arena);
    case EL_AST_STMT_CONTINUE:
        if (binder->loop_depth <= 0) {
            el_diag_report(
                binder->diag, EL_DIAG_ERROR, "sema.continue-outside-loop",
                in->span, "'continue' can only be used inside loops.",
            );
            return NULL;
        }
        return el_hir_new_continue_stmt(binder->hir_arena);

    case EL_AST_STMT_ASSIGN:
        return _el_binder_bind_assign(binder, in, &in->as.assign);
    case EL_AST_STMT_COMPOUND_ASSIGN:
        return _el_binder_bind_compound_assign(binder, in, &in->as.cassign);
    case EL_AST_STMT_VAR_DEF:
        return _el_binder_bind_var_definition(binder, in);
    }
    EL_UNREACHABLE_ENUM_VAL(ElAstStmtType, in->type);
}
