#include <elash/binder/binder.h>
#include <elash/util/assert.h>

#include <elash/diag/engine.h>
#include <elash/diag/meta.h>

#include <elash/sema/type/prim.h>

ElHirExprNode* _el_binder_bind_bin_expr(ElBinder* binder, ElAstExprNode* in, ElAstBinExprNode* bin) {
    ElHirExprNode* left  = el_binder_bind_expr(binder, bin->left);
    ElHirExprNode* right = el_binder_bind_expr(binder, bin->right);

    // TODO: simplified for now
    if (!el_sema_type_eql(left->type, right->type)) {
         el_diag_report(
            binder->diag, EL_DIAG_ERROR, "sema.type-mismatch",
            in->span,
            "invalid expression"
        );
    }

    ElType* type = left->type;
    if (el_sema_bin_op_is_comparison(bin->op)) {
        type = binder->type_bool;
    }

    return el_hir_new_bin_expr(binder->arena, type, bin->op, left, right);
}

ElHirExprNode* _el_binder_bind_unary_expr(ElBinder* binder, ElAstUnaryExprNode* unary) {
    ElHirExprNode* operand = el_binder_bind_expr(binder, unary->operand);
    if (!operand) return NULL;

    // TODO: implement type checking 
    return el_hir_new_unary_expr(binder->arena, operand->type, unary->op, operand);
}

ElHirExprNode* _el_binder_bind_literal(ElBinder* binder, ElAstExprNode* in, ElAstLiteralNode* lit) {
    switch (lit->type) {
    case EL_AST_LIT_INT:
        return el_hir_new_int_literal(binder->arena, binder->type_int, lit->of.int_.value);
    case EL_AST_LIT_CHAR:
        return el_hir_new_char_literal(binder->arena, binder->type_char, lit->of.char_.value);
    case EL_AST_LIT_BOOL:
        return el_hir_new_bool_literal(binder->arena, binder->type_bool, lit->of.bool_.value);
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

ElHirExprNode* _el_binder_bind_ident(ElBinder* binder, ElAstExprNode* in, ElAstIdentNode* ident) {
    ElSymbol* sym = el_sema_scope_lookup(binder->current_scope, ident->name);
    if (sym == NULL) {
        el_diag_report(
            binder->diag, EL_DIAG_ERROR, "sema.undefined-symbol",
            in->span,
            "undefined symbol '${name}'",
            EL_DIAG_STRING("name", ident->name)
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

ElHirExprNode* _el_binder_bind_call(ElBinder* binder, ElAstExprNode* in, ElAstCallExprNode* call) {
    ElHirExprNode* callee = el_binder_bind_expr(binder, call->callee);
    if (!callee) return NULL;

    if (callee->type->kind != EL_TYPE_FUNC) {
        el_diag_report(
            binder->diag, EL_DIAG_ERROR, "sema.not-callable",
            call->callee->span,
            "expression is not callable"
        );
        return NULL;
    }

    ElFunctionType* func = &callee->type->as.func;

    if (call->arg_count != func->param_count) {
        el_diag_report(
            binder->diag, EL_DIAG_ERROR, "sema.arg-count-mismatch",
            in->span,
            "expected ${expected} arguments, but got ${got}",
            EL_DIAG_INT("expected", func->param_count),
            EL_DIAG_INT("got", call->arg_count)
        );
        return NULL;
    }

    ElHirExprNode** args = EL_DYNARENA_NEW_ARR(binder->arena, ElHirExprNode*, call->arg_count);
    usize i = 0;
    for (ElAstExprNode* curr = call->args; curr != NULL; curr = curr->next) {
        args[i] = el_binder_bind_expr(binder, curr);
        if (!args[i]) return NULL;
        // TODO: implement argument type checking
        i++;
    }

    return el_hir_new_call_expr(binder->arena, func->ret_type, callee, args, call->arg_count);
}

ElHirExprNode* el_binder_bind_expr(ElBinder* binder, ElAstExprNode* in) {
    if (in == NULL) return NULL;

    switch (in->type) {
    case EL_AST_EXPR_BINARY:  return _el_binder_bind_bin_expr(binder, &in->as.binary);
    case EL_AST_EXPR_UNARY:   return _el_binder_bind_unary_expr(binder, &in->as.unary);
    case EL_AST_EXPR_LITERAL: return _el_binder_bind_literal(binder, in, &in->as.literal);
    case EL_AST_EXPR_IDENT:   return _el_binder_bind_ident(binder, in, &in->as.ident);
    case EL_AST_EXPR_CALL:    return _el_binder_bind_call(binder, in, &in->as.call);
    }
    EL_UNREACHABLE_ENUM_VAL(ElAstExprType, in->type);
}

