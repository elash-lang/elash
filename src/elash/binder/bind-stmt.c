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

ElHirStmtNode* el_binder_bind_stmt(ElBinder* binder, ElAstStmtNode* in) {
    switch (in->type) {
    case EL_AST_STMT_BLOCK: {
        ElHirBlockStmtNode block = _el_binder_bind_block(binder, &in->as.block);
        return el_hir_new_block_stmt(binder->arena, block.stmts);
    }
    case EL_AST_STMT_RETURN: {
        ElHirExprNode* val = el_binder_bind_expr(binder, in->as.return_.value);
        return el_hir_new_return_stmt(binder->arena, val);
    }
    case EL_AST_STMT_EXPR: {
        ElHirExprNode* expr = el_binder_bind_expr(binder, in->as.expr);
        return el_hir_new_expr_stmt(binder->arena, expr);
    }
    case EL_AST_STMT_IF: {
        EL_TODO("Implement binding for if statement");
    }
    case EL_AST_STMT_VAR_DEF: {
        ElType* type = _el_binder_bind_type(binder, in->as.var_def.type);
        if (!type) return NULL;

        ElSymbol* sym = el_sema_new_var_symbol(binder->arena, binder->sym_id_counter++, in->as.var_def.name->name, type);
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

        return el_hir_new_var_def_stmt(binder->arena, sym, init);
    }
    }
    EL_UNREACHABLE_ENUM_VAL(ElAstStmtType, in->type);
}

