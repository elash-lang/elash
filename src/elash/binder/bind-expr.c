#include <elash/binder/binder.h>
#include <elash/util/assert.h>

#include <elash/diag/engine.h>
#include <elash/diag/meta.h>

#include <elash/hir/type/prim.h>

#define IMPLICIT_CAST_IF_NEEDED(THING, SPAN, TO) \
    if (THING->type == NULL) { \
        THING = _el_binder_implicit_cast(binder, SPAN, THING, TO); \
        if (THING == NULL) return NULL; \
    }

#define REPORT_NON_INDEXABLE do { \
    return el_diag_report( \
        binder->diag, EL_DIAG_ERROR, "sema.non-indexable", \
        bin->left->span, "cannot index into non-array, non-slice, or non-raw-slice type" \
    ); \
} while (0)

ElHirExpr* _el_binder_bind_bin_expr(ElBinder* binder, ElAstExpr* in, ElAstBinExpr* bin) {
    ElHirExpr* left  = el_binder_bind_expr(binder, bin->left);
    ElHirExpr* right = el_binder_bind_expr(binder, bin->right);
    if (left == NULL || right == NULL) return NULL;

    ElHirType* type = left->type;
    if (bin->op != EL_SEMA_BIN_OP_INDEX) {
        if (el_sema_bin_op_is_logical(bin->op)) {
            IMPLICIT_CAST_IF_NEEDED(left, bin->left->span, binder->builtins->type_bool);
            IMPLICIT_CAST_IF_NEEDED(right, bin->right->span, binder->builtins->type_bool);
            if (left == NULL || right == NULL)
                return el_diag_report(
                    binder->diag, EL_DIAG_ERROR, "sema.type-mismatch",
                    in->span, "logical operator operands must be booleans"
                );

            type = binder->builtins->type_bool;
        } else {
            if (left->type == NULL && right->type != NULL) {
                left = _el_binder_implicit_cast(binder, bin->left->span, left, right->type);
                if (left == NULL) return NULL;
                type = left->type;
            } else if (right->type == NULL && left->type != NULL) {
                right = _el_binder_implicit_cast(binder, bin->right->span, right, left->type);
                if (right == NULL) return NULL;
                type = left->type;
            }

            if (!el_hir_type_eql(left->type, right->type))
                return el_diag_report(
                    binder->diag, EL_DIAG_ERROR, "sema.type-mismatch",
                    in->span, "left and right operand types must be identical"
                );

            if (el_sema_bin_op_is_comparison(bin->op)) {
                if (left->type == NULL) type = NULL;
                else type = binder->builtins->type_bool;
            }
        }
    } else {
        IMPLICIT_CAST_IF_NEEDED(right, bin->right->span, binder->builtins->type_usize);
        if (!el_hir_type_eql(right->type, binder->builtins->type_usize))
            return el_diag_report(
                binder->diag, EL_DIAG_ERROR, "sema.index-type",
                bin->right->span, "index expression must be of type compatible with usize"
            );

        if (left->type == NULL) REPORT_NON_INDEXABLE;
        switch (left->type->kind) {
        case EL_HIR_TYPE_ARRAY:   type = left->type->as.array.base;   break;
        case EL_HIR_TYPE_SLICE:   type = left->type->as.slice.base;   break;
        case EL_HIR_TYPE_RWSLICE: type = left->type->as.rwslice.base; break;
        default:                  REPORT_NON_INDEXABLE;               break;
        }
    }

    return el_hir_new_bin_expr(binder->hir_arena, type, bin->op, left, right);
}

ElHirExpr* _el_binder_bind_unary_expr(ElBinder* binder, ElAstExpr* in, ElAstUnaryExpr* unary) {
    ElHirExpr* operand = el_binder_bind_expr(binder, unary->operand);
    if (!operand) return NULL;

    ElHirType* type = operand->type;
    if (unary->op == EL_SEMA_UNARY_OP_NOT) {
        if (operand->type == NULL) {
            operand = _el_binder_implicit_cast(binder, unary->operand->span, operand, binder->builtins->type_bool);
            if (operand == NULL) return NULL;
        }
        if (!el_hir_type_eql(operand->type, binder->builtins->type_bool))
            return el_diag_report(
                binder->diag, EL_DIAG_ERROR, "sema.type-mismatch",
                in->span, "operand of logical NOT must be boolean"
            );
        type = binder->builtins->type_bool;
    } else if (type == NULL) {
        if (unary->op == EL_SEMA_UNARY_OP_ADDROF || unary->op == EL_SEMA_UNARY_OP_DEREF)
            return el_diag_report(
                binder->diag, EL_DIAG_ERROR, "sema.invalid-op",
                in->span, "cannot perform address-of or dereference on an untyped literal"
            );
    } else {
        if (unary->op == EL_SEMA_UNARY_OP_ADDROF) {
            type = el_hir_new_ref_type(binder->type_arena, operand->type);
        } else if (unary->op == EL_SEMA_UNARY_OP_DEREF) {
            if (operand->type->kind != EL_HIR_TYPE_REF)
                return el_diag_report(
                    binder->diag, EL_DIAG_ERROR, "sema.type-mismatch",
                    in->span, "cannot dereference non-pointer type ${type}",
                    EL_DIAG_STRING("type", EL_SV("TODO"))
                );
            type = operand->type->as.ref.base;
        }
    }

    return el_hir_new_unary_expr(binder->hir_arena, type, unary->op, operand);
}

ElHirExpr* _el_binder_bind_literal(ElBinder* binder, ElAstExpr* in, ElAstLiteral* lit) {
    switch (lit->type) {
    case EL_AST_LIT_INT:
        return el_hir_new_untyped_int_lit(binder->hir_arena, lit->of.int_.value);
    case EL_AST_LIT_CHAR:
        return el_hir_new_untyped_char_lit(binder->hir_arena, lit->of.char_.value);
    case EL_AST_LIT_BOOL:
        return el_hir_new_untyped_bool_lit(binder->hir_arena, lit->of.bool_.value);
    default:
        // TODO: handle other literal types
        return el_diag_report(
            binder->diag, EL_DIAG_ERROR, "sema.unsupported-literal",
            in->span,
            "unsupported literal type"
        );
    }
}

ElHirExpr* _el_binder_bind_ident(ElBinder* binder, ElAstExpr* in, ElAstIdent* ident) {
    ElHirSymbol* sym = el_hir_scope_lookup(binder->current_scope, ident->name);
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
        return el_diag_report(
            binder->diag, EL_DIAG_ERROR, "sema.type-used-as-expr",
            in->span,
            "symbol '${name}' declared as type but used as an expression",
            EL_DIAG_STRING("name", sym->name)
        );
    }
    return NULL;
}

ElHirExpr* _el_binder_bind_call(ElBinder* binder, ElAstExpr* in, ElAstCallExpr* call) {
    ElHirExpr* callee = el_binder_bind_expr(binder, call->callee);
    if (!callee) return NULL;

    if (callee->kind == EL_HIR_EXPR_SYMBOL && callee->as.symbol->kind == EL_SYM_BUILTIN) {
        return el_binder_bind_builtin_call(binder, in, call, callee->as.symbol);
    }

    if (callee->type == NULL || callee->type->kind != EL_HIR_TYPE_FUNC)
        return el_diag_report(
            binder->diag, EL_DIAG_ERROR, "sema.not-callable",
            call->callee->span,
            "expression is not callable"
        );

    ElHirFuncType* func = &callee->type->as.func;
    if (call->arg_count != func->param_count)
        return el_diag_report(
            binder->diag, EL_DIAG_ERROR, "sema.arg-count-mismatch",
            in->span, "expected ${expected} arguments, but got ${got}",
            EL_DIAG_INT("expected", func->param_count),
            EL_DIAG_INT("got", call->arg_count)
        );

    ElHirExpr** args = EL_DYNARENA_NEW_ARR(binder->hir_arena, ElHirExpr*, call->arg_count);
    usize i = 0;
    for (ElAstInit* curr = call->args; curr != NULL; curr = curr->next) {
        args[i] = el_binder_bind_init(binder, curr, func->params[i]);
        if (!args[i]) return NULL;
        i++;
    }

    return el_hir_new_call_expr(binder->hir_arena, func->ret_type, callee, args, call->arg_count);
}

ElHirExpr* _el_binder_bind_cast(ElBinder* binder, ElAstExpr* in, ElAstCastExpr* cast) {
    ElHirExpr* expr = el_binder_bind_expr(binder, cast->expr);
    ElHirType*    type = _el_binder_bind_type(binder, cast->type);

    return _el_binder_explicit_cast(binder, in->span, expr, type);
}

ElHirExpr* _el_binder_bind_array_lit(ElBinder* binder, ElAstExpr* _, ElAstArrayLit* array_lit) {
    ElHirType* type = _el_binder_bind_type(binder, array_lit->type);
    if (type == NULL) return NULL;

    return el_binder_bind_init(binder, array_lit->init, type);
}

ElHirExpr* _el_binder_bind_expr_impl(ElBinder* binder, ElAstExpr* in) {
    if (in == NULL) return NULL;

    switch (in->type) {
    case EL_AST_EXPR_BINARY:   return _el_binder_bind_bin_expr(binder, in, &in->as.binary);
    case EL_AST_EXPR_UNARY:    return _el_binder_bind_unary_expr(binder, in, &in->as.unary);
    case EL_AST_EXPR_LITERAL:  return _el_binder_bind_literal(binder, in, &in->as.literal);
    case EL_AST_EXPR_IDENT:    return _el_binder_bind_ident(binder, in, &in->as.ident);
    case EL_AST_EXPR_CALL:     return _el_binder_bind_call(binder, in, &in->as.call);
    case EL_AST_EXPR_CAST:     return _el_binder_bind_cast(binder, in, &in->as.cast);
    case EL_AST_EXPR_ARRAYLIT: return _el_binder_bind_array_lit(binder, in, &in->as.array_lit);
    }
    EL_UNREACHABLE_ENUM_VAL(ElAstExprType, in->type);
}

ElHirExpr* el_binder_bind_expr(ElBinder* binder, ElAstExpr* in) {
    ElHirExpr* expr = _el_binder_bind_expr_impl(binder, in);
    if (expr == NULL) return NULL;
    return _el_binder_simplify_expr(binder, expr);
}
