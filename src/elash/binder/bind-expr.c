#include <elash/binder/binder.h>
#include <elash/util/assert.h>

#include <elash/diag/engine.h>
#include <elash/diag/meta.h>

#include <elash/sema/type/prim.h>

// TODO: this function is too large and probably should be splitted into smaller helpers
ElHirExprNode* el_binder_bind_expr(ElBinder* binder, ElAstExprNode* in) {
    if (in == NULL) return NULL;

    switch (in->type) {
    case EL_AST_EXPR_BINARY: {
        ElHirExprNode* left = el_binder_bind_expr(binder, in->as.binary.left);
        ElHirExprNode* right = el_binder_bind_expr(binder, in->as.binary.right);
        if (!left || !right) return NULL;

        // TODO: implement type checking 
        return el_hir_new_bin_expr(binder->arena, left->type, in->as.binary.op, left, right);
    }
    case EL_AST_EXPR_UNARY: {
        ElHirExprNode* operand = el_binder_bind_expr(binder, in->as.unary.operand);
        if (!operand) return NULL;

        // TODO: implement type checking 
        return el_hir_new_unary_expr(binder->arena, operand->type, in->as.unary.op, operand);
    }
    case EL_AST_EXPR_LITERAL: {
        switch (in->as.literal.type) {
        case EL_AST_LIT_INT:
            return el_hir_new_int_literal(binder->arena, binder->type_int, in->as.literal.of.int_.value);
        case EL_AST_LIT_CHAR:
            return el_hir_new_char_literal(binder->arena, binder->type_char, in->as.literal.of.char_.value);
        case EL_AST_LIT_BOOL:
            return el_hir_new_bool_literal(binder->arena, binder->type_bool, in->as.literal.of.bool_.value);
        default:
            // TODO: handle other literal types
            el_diag_report(
                binder->diag, EL_DIAG_ERROR, "sema.unsupported-literal",
                in->span,
                "unsupported literal type"
            );
            return NULL;
        }
    }
    case EL_AST_EXPR_IDENT: {
        ElSymbol* sym = el_sema_scope_lookup(binder->current_scope, in->as.ident.name);
        if (sym == NULL) {
            el_diag_report(
                binder->diag, EL_DIAG_ERROR, "sema.undefined-symbol",
                in->span,
                "undefined symbol '${name}'",
                EL_DIAG_STRING("name", in->as.ident.name)
            );
            return NULL;
        }

        switch (sym->kind) {
        case EL_SYM_VAR:
            return el_hir_new_symbol_expr(binder->arena, sym->as.var.type, sym);
        case EL_SYM_FUNC:
            return el_hir_new_symbol_expr(binder->arena, sym->as.func.type, sym);
        case EL_SYM_TYPE:
            el_diag_report(
                binder->diag, EL_DIAG_ERROR, "sema.type-used-as-expr",
                in->span,
                "symbol '${name}' declared as type but used as an expression",
                EL_DIAG_STRING("name", sym->name)
            );
            return NULL;
        }
        return NULL;
    }
    case EL_AST_EXPR_CALL: {
        ElHirExprNode* callee = el_binder_bind_expr(binder, in->as.call.callee);
        if (!callee) return NULL;

        if (callee->type->kind != EL_TYPE_FUNC) {
            el_diag_report(
                binder->diag, EL_DIAG_ERROR, "sema.not-callable",
                in->as.call.callee->span,
                "expression is not callable"
            );
            return NULL;
        }

        ElFunctionType* func = &callee->type->as.func;

        if (in->as.call.arg_count != func->param_count) {
            el_diag_report(
                binder->diag, EL_DIAG_ERROR, "sema.arg-count-mismatch",
                in->span,
                "expected ${expected} arguments, but got ${got}",
                EL_DIAG_INT("expected", func->param_count),
                EL_DIAG_INT("got", in->as.call.arg_count)
            );
            return NULL;
        }

        ElHirExprNode** args = EL_DYNARENA_NEW_ARR(binder->arena, ElHirExprNode*, in->as.call.arg_count);
        usize i = 0;
        for (ElAstExprNode* curr = in->as.call.args; curr != NULL; curr = curr->next) {
            args[i] = el_binder_bind_expr(binder, curr);
            if (!args[i]) return NULL;
            // TODO: implement argument type checking
            i++;
        }

        return el_hir_new_call_expr(binder->arena, func->ret_type, callee, args, in->as.call.arg_count);
    }
    }
    EL_UNREACHABLE_ENUM_VAL(ElAstExprType, in->type);
}

