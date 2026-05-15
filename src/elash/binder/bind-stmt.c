#include <elash/binder/binder.h>
#include <elash/util/assert.h>
#include <elash/util/todo.h>

#include <elash/diag/engine.h>
#include <elash/diag/meta.h>

#include <elash/hir/tree/stmt/block.h>
#include <elash/hir/tree/stmt/return.h>

ElHirBlockStmtNode _el_binder_bind_block(ElBinder* binder, ElAstBlockStmtNode* in) {
    ElHirStmtNode* head = NULL;
    ElHirStmtNode* tail = NULL;

    for (ElAstStmtNode* curr = in->stmts; curr != NULL; curr = curr->next) {
        ElHirStmtNode* binded = el_binder_bind_stmt(binder, curr);
        if (binded) {
            el_hir_stmt_list_append(&head, &tail, binded);
        }
    }

    return (ElHirBlockStmtNode) { .stmts = head };
}

ElHirStmtNode* _el_binder_bind_return(ElBinder* binder, ElAstStmtNode* in) {
    ElHirExprNode* val = el_binder_bind_expr(binder, in->as.return_.value);
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

ElHirStmtNode* _el_binder_bind_var_definition(ElBinder* binder, ElAstStmtNode* in) {
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

    ElHirExprNode* init = NULL;
    if (in->as.var_def.init) {
        init = el_binder_bind_expr(binder, in->as.var_def.init);
        if (!init) return NULL;
        // TODO: implement type checking 
    }

    return el_hir_new_var_def_stmt(binder->hir_arena, sym, init);
}

ElHirStmtNode* el_binder_bind_stmt(ElBinder* binder, ElAstStmtNode* in) {
    switch (in->type) {
    case EL_AST_STMT_BLOCK: {
        ElHirBlockStmtNode block = _el_binder_bind_block(binder, &in->as.block);
        return el_hir_new_block_stmt(binder->hir_arena, block.stmts);
    }
    case EL_AST_STMT_RETURN:
        return _el_binder_bind_return(binder, in);
    case EL_AST_STMT_EXPR: {
        ElHirExprNode* expr = el_binder_bind_expr(binder, in->as.expr);
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
        return el_hir_new_while_stmt(
            binder->hir_arena,
            el_binder_bind_expr(binder, in->as.while_.cond),
            el_binder_bind_stmt(binder, in->as.while_.body)
        );

    case EL_AST_STMT_ASSIGN:
        return el_hir_new_assign_stmt(
            binder->hir_arena,
            el_binder_bind_expr(binder, in->as.assign.target),
            el_binder_bind_expr(binder, in->as.assign.value)
        );

    case EL_AST_STMT_VAR_DEF:
        return _el_binder_bind_var_definition(binder, in);
    }
    EL_UNREACHABLE_ENUM_VAL(ElAstStmtType, in->type);
}

