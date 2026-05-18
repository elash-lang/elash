#include <elash/binder/binder.h>
#include <elash/util/assert.h>

#include <elash/diag/engine.h>
#include <elash/diag/meta.h>

#include <elash/sema/type/prim.h>

ElHirExpr* _el_binder_bind_bin_expr(ElBinder* binder, ElAstExpr* in, ElAstBinExpr* bin) {
    ElHirExpr* left  = el_binder_bind_expr(binder, bin->left);
    ElHirExpr* right = el_binder_bind_expr(binder, bin->right);
    if (left == NULL || right == NULL) return NULL;

    ElType* type = left->type;
    if (bin->op != EL_SEMA_BIN_OP_INDEX) {
        if (!el_sema_type_eql(left->type, right->type)) {
            el_diag_report(
                binder->diag, EL_DIAG_ERROR, "sema.type-mismatch",
                in->span,
                "left and right operand types must be identical"
            );
            return NULL;
        }

        if (el_sema_bin_op_is_comparison(bin->op)) {
            type = binder->builtins->type_bool;
        }
    } else {
        if (!el_sema_type_eql(right->type, binder->builtins->type_int)) {
            el_diag_report(
                binder->diag, EL_DIAG_ERROR, "sema.index-type",
                bin->right->span, "index expression must be integer",
            );
            return NULL;
        }

        ElTypeKind k = left->type->kind;
        if (k != EL_TYPE_ARRAY && k != EL_TYPE_SLICE && k != EL_TYPE_RAW_SLICE) {
            el_diag_report(
                binder->diag, EL_DIAG_ERROR, "sema.sema.non-indexable",
                bin->left->span, "cannot index into non-array, non-slice, or non-raw-slice type",
            );
        }

        if (k == EL_TYPE_RAW_SLICE)  type = left->type->as.raw_slice.base;
        else if (k == EL_TYPE_ARRAY) type = left->type->as.array.base;
        else if (k == EL_TYPE_SLICE) type = left->type->as.slice.base;
    }

    return el_hir_new_bin_expr(binder->hir_arena, type, bin->op, left, right);
}

ElHirExpr* _el_binder_bind_unary_expr(ElBinder* binder, ElAstExpr* in, ElAstUnaryExpr* unary) {
    ElHirExpr* operand = el_binder_bind_expr(binder, unary->operand);
    if (!operand) return NULL;

    ElType* type = operand->type;
    if (unary->op == EL_SEMA_UNARY_OP_ADDROF) {
        type = el_sema_new_ptr_type(binder->type_arena, operand->type);
    } else if (unary->op == EL_SEMA_UNARY_OP_DEREF) {
        if (operand->type->kind != EL_TYPE_PTR) {
            el_diag_report(
                binder->diag, EL_DIAG_ERROR, "sema.type-mismatch",
                in->span,
                "cannot dereference non-pointer type ${type}",
                EL_DIAG_STRING("type", EL_SV("TODO"))
            );
            return NULL;
        }
        type = operand->type->as.ptr.base;
    }

    return el_hir_new_unary_expr(binder->hir_arena, type, unary->op, operand);
}

ElHirExpr* _el_binder_bind_literal(ElBinder* binder, ElAstExpr* in, ElAstLiteral* lit) {
    switch (lit->type) {
    case EL_AST_LIT_INT:
        return el_hir_new_int_literal(binder->hir_arena, binder->builtins->type_int, lit->of.int_.value);
    case EL_AST_LIT_CHAR:
        return el_hir_new_char_literal(binder->hir_arena, binder->builtins->type_char, lit->of.char_.value);
    case EL_AST_LIT_BOOL:
        return el_hir_new_bool_literal(binder->hir_arena, binder->builtins->type_bool, lit->of.bool_.value);
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

ElHirExpr* _el_binder_bind_ident(ElBinder* binder, ElAstExpr* in, ElAstIdent* ident) {
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
        return el_hir_new_symbol_expr(binder->hir_arena, sym->as.var.type, sym);
    case EL_SYM_FUNC:
        return el_hir_new_symbol_expr(binder->hir_arena, sym->as.func.type, sym);
    case EL_SYM_BUILTIN:
        return el_hir_new_symbol_expr(binder->hir_arena, binder->builtins->type_void, sym);
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

ElHirExpr* _el_binder_bind_call(ElBinder* binder, ElAstExpr* in, ElAstCallExpr* call) {
    ElHirExpr* callee = el_binder_bind_expr(binder, call->callee);
    if (!callee) return NULL;

    if (callee->kind == EL_HIR_EXPR_SYMBOL && callee->as.symbol->kind == EL_SYM_BUILTIN) {
        return el_binder_bind_builtin_call(binder, in, call, callee->as.symbol);
    }

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

    ElHirExpr** args = EL_DYNARENA_NEW_ARR(binder->hir_arena, ElHirExpr*, call->arg_count);
    usize i = 0;
    for (ElAstExpr* curr = call->args; curr != NULL; curr = curr->next) {
        args[i] = el_binder_bind_expr(binder, curr);
        if (!args[i]) return NULL;
        // TODO: implement argument type checking
        i++;
    }

    return el_hir_new_call_expr(binder->hir_arena, func->ret_type, callee, args, call->arg_count);
}

ElHirExpr* _el_binder_bind_array_lit(ElBinder* binder, ElAstExpr* _, ElAstArrayLit* array_lit) {
    ElType* type = _el_binder_bind_type(binder, array_lit->type);
    if (type == NULL) return NULL;

    return el_binder_bind_init(binder, array_lit->init, type);
}

ElHirExpr* el_binder_bind_expr(ElBinder* binder, ElAstExpr* in) {
    if (in == NULL) return NULL;

    switch (in->type) {
    case EL_AST_EXPR_BINARY:        return _el_binder_bind_bin_expr(binder, in, &in->as.binary);
    case EL_AST_EXPR_UNARY:         return _el_binder_bind_unary_expr(binder, in, &in->as.unary);
    case EL_AST_EXPR_LITERAL:       return _el_binder_bind_literal(binder, in, &in->as.literal);
    case EL_AST_EXPR_IDENT:         return _el_binder_bind_ident(binder, in, &in->as.ident);
    case EL_AST_EXPR_CALL:          return _el_binder_bind_call(binder, in, &in->as.call);
    case EL_AST_EXPR_ARRAY_LITERAL: return _el_binder_bind_array_lit(binder, in, &in->as.array_lit);
    }
    EL_UNREACHABLE_ENUM_VAL(ElAstExprType, in->type);
}
