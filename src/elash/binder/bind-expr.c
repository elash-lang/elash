#include "binder-internals.h"

#include <elash/diag/engine.h>
#include <elash/util/assert.h>
#include <elash/util/todo.h>

#include <elash/hir/tree/expr.h>
#include <elash/hir/type/prim.h>
#include <elash/hir/type/opt.h>
#include <elash/hir/tree/expr/intr.h>
#include <elash/ast/tree/toi.h>

#define IMPLICIT_CAST_IF_NEEDED(THING, SPAN, TO)                           \
    if ((THING)->type == NULL) {                                           \
        (THING) = _el_binder_implicit_cast(binder, (SPAN), (THING), (TO)); \
        if ((THING) == NULL) return NULL;                                  \
    }

#define REPORT_NON_INDEXABLE do {                                                        \
    return el_diag_report(                                                               \
        binder->diag, EL_DIAG_ERROR, "sema.non-indexable",                               \
        bin->left->span, "cannot index into non-array, non-slice, or non-raw-slice type" \
    );                                                                                   \
} while (0)

static bool is_distinct_related(ElHirType* a, ElHirType* b) {
    if (a->kind == EL_HIR_TYPE_DISTINCT && el_hir_type_eql(a->as.distinct.orig, b)) return true;
    if (b->kind == EL_HIR_TYPE_DISTINCT && el_hir_type_eql(b->as.distinct.orig, a)) return true;
    return false;
}

static bool is_null_lit(const ElHirExpr* expr) {
    return expr->kind == EL_HIR_EXPR_LITERAL && expr->as.literal.kind == EL_HIR_LITERAL_NULL;
}

static bool types_opt_base_match(const ElHirType* opt, const ElHirType* other) {
    if (opt->kind != EL_HIR_TYPE_OPT) return false;

    ElHirType* base = opt->as.opt.base;
    if (el_hir_type_eql(base, other)) return true;

    return base->kind == EL_HIR_TYPE_REF && other->kind == EL_HIR_TYPE_REF
        && el_hir_type_eql(base->as.ref.base, other->as.ref.base);
}

static bool bind_opt_side(
    ElBinder* binder, ElSourceSpan span, ElHirExpr* opt, ElHirExpr** other
) {
    if (opt->type == NULL)                  return false;
    if (opt->type->kind != EL_HIR_TYPE_OPT) return false;

    ElHirType* base = opt->type->as.opt.base;
    if ((*other)->type == NULL) {
        *other = _el_binder_implicit_cast(binder, span, *other, base);
        if (*other == NULL) return false;
    }

    if (el_hir_type_eql(base, (*other)->type)) {
        return true;
    }

    if ((*other)->type != NULL && !el_hir_type_eql((*other)->type, opt->type)) {
        ElHirExpr* casted = _el_binder_implicit_cast(binder, span, *other, base);
        if (casted != NULL) {
            *other = casted;
            return true;
        }
    }

    if (base->kind != EL_HIR_TYPE_REF)           return false;
    if ((*other)->type == NULL)                  return false;
    if ((*other)->type->kind != EL_HIR_TYPE_REF) return false;

    return el_hir_type_eql(base->as.ref.base, (*other)->type->as.ref.base);
}

static ElHirExpr* bind_optional_eql(
    ElBinder* binder, ElAstExpr* in, ElAstBinExpr* bin, ElHirExpr* left, ElHirExpr* right
) {
    // untyped null -> null as T?
    if (is_null_lit(left) && right->type != NULL && right->type->kind == EL_HIR_TYPE_OPT) {
        left = _el_binder_implicit_cast(binder, bin->left->span, left, right->type);
        if (left == NULL) return NULL;
    }
    if (is_null_lit(right) && left->type != NULL && left->type->kind == EL_HIR_TYPE_OPT) {
        right = _el_binder_implicit_cast(binder, bin->right->span, right, left->type);
        if (right == NULL) return NULL;
    }

    bool compatible = false;
    if (left->type != NULL && right->type != NULL) {
        compatible |= types_opt_base_match(left->type, right->type)
                   || types_opt_base_match(right->type, left->type);
    }

    if (!compatible) {
        compatible |= bind_opt_side(binder, bin->right->span, left, &right)
                   || bind_opt_side(binder, bin->left->span, right, &left);
    }

    if (compatible) {
        return el_hir_new_bin_expr(
            binder->arena, in->span, binder->builtins->type_bool, bin->op, left, right
        );
    }

    return NULL;
}

static ElHirExpr* bind_optional_bin_op(
    ElBinder* binder, ElAstExpr* in, ElAstBinExpr* bin, ElHirExpr* left, ElHirExpr* right
) {
    if (left->type == NULL || left->type->kind != EL_HIR_TYPE_OPT) {
        return el_diag_report(
            binder->diag, EL_DIAG_ERROR, "sema.type-mismatch",
            bin->left->span, "left operand of optional operator must be an optional type"
        );
    }

    ElHirType* otype = left->type;
    ElHirType* base  = otype->as.opt.base;

    if (bin->op == EL_SEMA_BIN_OP_OPT_FB) {
        if (right->type == NULL) {
            right = _el_binder_implicit_cast(binder, bin->right->span, right, base);
        } else if (right->type->kind == EL_HIR_TYPE_OPT) {
            if (!el_hir_type_eql(right->type->as.opt.base, base)) {
                return el_diag_report(
                    binder->diag, EL_DIAG_ERROR, "sema.type-mismatch",
                    in->span, "optional fallback operands have incompatible types"
                );
            }
        } else if (!el_hir_type_eql(right->type, base)) {
            return el_diag_report(
                binder->diag, EL_DIAG_ERROR, "sema.type-mismatch",
                in->span, "optional fallback operands have incompatible types"
            );
        }
        if (right == NULL) return NULL;

        if (right->type->kind != EL_HIR_TYPE_OPT) {
            right = el_hir_new_some_opt_intr(binder->arena, bin->right->span, otype, right);
        }
        return el_hir_new_bin_expr(binder->arena, in->span, otype, bin->op, left, right);
    }

    if (right->type == NULL)
        right = _el_binder_apply_default_type(binder, right);

    ElHirType* result_type = el_hir_new_opt_type(binder->arena, right->type);
    return el_hir_new_bin_expr(binder->arena, in->span, result_type, bin->op, left, right);
}

static ElHirType* bind_arith_op(ElBinder* binder, ElAstExpr* in, ElAstBinExpr* bin, ElHirExpr** left, ElHirExpr** right) {
    ElHirType* type = (*left)->type;

    if ((*left)->type == NULL && (*right)->type != NULL) {
        (*left) = _el_binder_implicit_cast(binder, bin->left->span, *left, (*right)->type);
        if ((*left) == NULL) return NULL;
        type = (*left)->type;
    } else if ((*right)->type == NULL && (*left)->type != NULL) {
        (*right) = _el_binder_implicit_cast(binder, bin->right->span, *right, (*left)->type);
        if ((*right) == NULL) return NULL;
        type = (*left)->type;
    } else if ((*left)->type != NULL && (*right)->type != NULL && !el_hir_type_eql((*left)->type, (*right)->type)) {
        if (is_distinct_related((*left)->type, (*right)->type)) {
            if ((*left)->type->kind == EL_HIR_TYPE_DISTINCT)
                (*right) = _el_binder_implicit_cast(binder, bin->right->span, *right, (*left)->type);
            else
                (*left) = _el_binder_implicit_cast(binder, bin->left->span, *left, (*right)->type);

            if (*left == NULL || *right == NULL) return NULL;
            type = (*left)->type;
        }
    }

    if (!el_hir_type_eql((*left)->type, (*right)->type))
        return el_diag_report(
            binder->diag, EL_DIAG_ERROR, "sema.type-mismatch",
            in->span, "left and right hand side types are incompatible"
        );

    if (el_bin_op_is_comparison(bin->op)) {
        if ((*left)->type == NULL) type = NULL;
        else type = binder->builtins->type_bool;
    }

    return type;
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity): AGAIN, clang-tidy doesn't understand macros
ElHirExpr* _el_binder_bind_bin_expr(ElBinder* binder, ElAstExpr* in, ElAstBinExpr* bin) {
    ElHirExpr* left  = el_binder_bind_expr(binder, bin->left);
    ElHirExpr* right = el_binder_bind_expr(binder, bin->right);
    if (left == NULL || right == NULL) return NULL;

    if (el_bin_op_is_optional(bin->op))
        return bind_optional_bin_op(binder, in, bin, left, right);

    if (el_bin_op_is_equality(bin->op)) {
        ElHirExpr* result = bind_optional_eql(binder, in, bin, left, right);
        if (result != NULL) return result;
    }

    ElHirType* type = left->type;
    if (bin->op != EL_SEMA_BIN_OP_INDEX) {
        if (el_bin_op_is_logical(bin->op)) {
            IMPLICIT_CAST_IF_NEEDED(left, bin->left->span, binder->builtins->type_bool);
            IMPLICIT_CAST_IF_NEEDED(right, bin->right->span, binder->builtins->type_bool);
            if (left == NULL || right == NULL)
                return el_diag_report(
                    binder->diag, EL_DIAG_ERROR, "sema.type-mismatch",
                    in->span, "logical operator operands must be booleans"
                );

            type = binder->builtins->type_bool;
        } else {
            type = bind_arith_op(binder, in, bin, &left, &right);
            if (left == NULL || right == NULL) return NULL;
        }
    } else {
        IMPLICIT_CAST_IF_NEEDED(right, bin->right->span, binder->builtins->type_usize);
        if (!el_hir_type_eql(right->type, binder->builtins->type_usize))
            return el_diag_report(
                binder->diag, EL_DIAG_ERROR, "sema.index-type",
                bin->right->span, "index expression must be of type compatible with usize"
            );

        if (left->type == NULL) REPORT_NON_INDEXABLE;

        ElHirType* type_to_check = el_hir_type_unwrap_distinct(left->type);

        switch (type_to_check->kind) {
        case EL_HIR_TYPE_ARRAY:   type = type_to_check->as.array.base;   break;
        case EL_HIR_TYPE_SLICE:   type = type_to_check->as.slice.base;   break;
        case EL_HIR_TYPE_RWSLICE: type = type_to_check->as.rwslice.base; break;
        default:                  REPORT_NON_INDEXABLE;                  break;
        }
    }

    return el_hir_new_bin_expr(binder->arena, in->span, type, bin->op, left, right);
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
ElHirExpr* _el_binder_bind_unary_expr(ElBinder* binder, ElAstExpr* in, ElAstUnaryExpr* unary) {
    ElHirExpr* operand = el_binder_bind_expr(binder, unary->operand);
    if (operand == NULL) return NULL;

    ElHirType* type = operand->type;
    if (unary->op == EL_SEMA_UNARY_OP_NOT) {
        if (operand->type == NULL) {
            operand = _el_binder_implicit_cast(binder, unary->operand->span, operand, binder->builtins->type_bool);
            if (operand == NULL) return NULL;
        }
        if (!el_hir_type_eql(operand->type, binder->builtins->type_bool))
            return el_diag_report(
                binder->diag, EL_DIAG_ERROR, "sema.type-mismatch",
                unary->operand->span, "operand of logical NOT must be boolean"
            );
        type = binder->builtins->type_bool;
    } else if (unary->op == EL_SEMA_UNARY_OP_OPT_UNWRAP) {
        if (type == NULL || type->kind != EL_HIR_TYPE_OPT) {
            return el_diag_report(
                binder->diag, EL_DIAG_ERROR, "sema.type-mismatch",
                unary->operand->span, "operand of optional unwrap operator must be an optional"
            );
        }
        type = type->as.opt.base;
    } else if (type == NULL){
        if (unary->op == EL_SEMA_UNARY_OP_DEREF)
            return el_diag_report(
                binder->diag, EL_DIAG_ERROR, "sema.invalid-op",
                in->span, "cannot dereference an untyped literal"
            );
        if (unary->op == EL_SEMA_UNARY_OP_ADDROF)
            return el_diag_report(
                binder->diag, EL_DIAG_ERROR, "sema.invalid-op",
                in->span, "cannot perform address-of on an untyped literal"
            );
        if (el_unary_op_is_incdec(unary->op))
            return el_diag_report(
                binder->diag, EL_DIAG_ERROR, "sema.invalid-op",
                in->span, "cannot increment or decrement an untyped literal"
            );
    } else {
        if (unary->op == EL_SEMA_UNARY_OP_ADDROF) {
            if (!el_hir_expr_is_lvalue(operand))
                return el_diag_report(
                    binder->diag, EL_DIAG_ERROR, "sema.invalid-op",
                    in->span, "address-of operator requires an lvalue"
                );
            type = el_hir_new_ref_type(binder->arena, operand->type);
        } else if (el_unary_op_is_incdec(unary->op)) {
            if (!el_hir_expr_is_lvalue(operand))
                return el_diag_report(
                    binder->diag, EL_DIAG_ERROR, "sema.invalid-op",
                    in->span, "increment/decrement requires an lvalue"
                );
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

    return el_hir_new_unary_expr(binder->arena, in->span, type, unary->op, operand);
}

ElHirExpr* _el_binder_bind_literal(ElBinder* binder, ElAstExpr* in, ElAstLiteral* lit) {
    switch (lit->kind) {
    case EL_AST_LIT_INT:
        return el_hir_new_int_lit(binder->arena, in->span, lit->of.int_);
    case EL_AST_LIT_CHAR:
        return el_hir_new_char_lit(binder->arena, in->span, lit->of.char_);
    case EL_AST_LIT_BOOL:
        return el_hir_new_bool_lit(binder->arena, in->span, lit->of.bool_);
    case EL_AST_LIT_FLOAT:
        return el_hir_new_float_lit(binder->arena, in->span, lit->of.float_);
    case EL_AST_LIT_STRING:
        return el_hir_new_str_lit(binder->arena, in->span, lit->of.str_);
    case EL_AST_LIT_NULL:
        return el_hir_new_null_lit(binder->arena, in->span);
    }
    EL_UNREACHABLE_ENUM_VAL(ElAstLiteralKind, lit->kind);
}

ElHirExpr* _el_binder_bind_ident(ElBinder* binder, ElAstExpr* in, ElAstIdent* ident) {
    ElHirSymbol* sym = el_hir_scope_lookup(binder->current_scope, ident->name);
    if (sym == NULL) {
        el_diag_report(
            binder->diag, EL_DIAG_ERROR, "sema.undefined-symbol",
            in->span, "undefined symbol '${name}'",
            EL_DIAG_STRING("name", ident->name)
        );
        return NULL;
    }

    switch (sym->kind) {
    case EL_SYM_VAR:
        return el_hir_new_symbol_expr(binder->arena, in->span, sym->as.var.type, sym);
    case EL_SYM_FUNC:
        return el_hir_new_symbol_expr(binder->arena, in->span, sym->as.func.type, sym);
    case EL_SYM_BUILTIN:
        return el_hir_new_symbol_expr(binder->arena, in->span, binder->builtins->type_void, sym);
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

    ElHirExpr** args = EL_DYNARENA_NEW_ARR(binder->arena, ElHirExpr*, call->arg_count);
    usize i = 0;
    for (ElAstToI* curr = call->args; curr != NULL; curr = curr->next) {
        ElHirToE* toe = el_binder_bind_toi(binder, curr, func->params[i], EL_STORAGECLS_LOCAL);
        if (toe == NULL) return NULL;
        if (toe->is_type) {
            el_diag_report(
                binder->diag, EL_DIAG_ERROR, "sema.type-arg",
                curr->span, "cannot pass a type as an argument to a regular function"
            );
            el_diag_help(
                binder->diag, "types can only be passed to specific builtins, such as 'sizeof'"
            );
            return NULL;
        }

        args[i] = toe->as.expr;
        if (!args[i]) return NULL;
        i++;
    }

    return el_hir_new_call_expr(binder->arena, in->span, func->ret_type, callee, args, call->arg_count);
}

ElHirExpr* _el_binder_bind_cast(ElBinder* binder, ElAstExpr* in, ElAstCastExpr* cast) {
    ElHirExpr* expr = el_binder_bind_expr(binder, cast->expr);
    ElHirType* type = el_binder_bind_type(binder, cast->type);

    if (expr == NULL || type == NULL) return NULL;
    if (!_el_binder_ensure_complete(binder, cast->type->span, type))
        return NULL;

    expr = _el_binder_apply_default_type(binder, expr);
    if (expr == NULL) return NULL;

    if (cast->kind == EL_SEMCAST) {
        return _el_binder_explicit_cast(binder, in->span, expr, type);
    } else {
        usize ssize = _el_binder_sizeof(binder, expr->type);
        usize dsize = _el_binder_sizeof(binder, type);
        if (ssize != dsize) {
            return el_diag_report(
                binder->diag, EL_DIAG_ERROR, "sema.invalid-bitcast",
                expr->span, "bitcast source size does not match destination size (${source} vs ${dest} bytes)",
                EL_DIAG_INT("source", ssize), EL_DIAG_INT("dest", dsize),
            );
        }

        return el_hir_new_bitcast_expr(binder->arena, expr->span, type, expr);
    }
}

ElHirExpr* el_binder_bind_typedinit(ElBinder* binder, ElAstExpr* in, ElAstTypedInit* tinit) {
    (void) in;
    ElHirType* type = el_binder_bind_type(binder, tinit->type);
    if (type == NULL) return NULL;

    return el_binder_bind_init(binder, tinit->init, type, tinit->scls);
}

ElHirExpr* _el_binder_bind_member_expr(ElBinder* binder, ElAstExpr* in, ElAstMemberExpr* member) {
    ElHirExpr* expr = el_binder_bind_expr(binder, member->expr);
    if (expr == NULL) return NULL;

    ElHirType* type = expr->type;
    if (type != NULL) type = el_hir_type_unwrap_distinct(type);

    if (member->is_optional) {
        if (type == NULL || type->kind != EL_HIR_TYPE_OPT) {
            return el_diag_report(
                binder->diag, EL_DIAG_ERROR, "sema.type-mismatch",
                member->expr->span, "optional member access requires an optional operand"
            );
        }
        type = el_hir_type_unwrap_distinct(type->as.opt.base);
    }

    if (type == NULL || type->kind != EL_HIR_TYPE_STRUCT) {
        el_diag_report(
            binder->diag, EL_DIAG_ERROR, "sema.not-struct",
            member->expr->span, "member access requires a struct value"
        );
        if (type != NULL && type->kind == EL_HIR_TYPE_REF) {
            const ElHirType* base = el_hir_type_unwrap_distinct(type->as.ref.base);
            if (base != NULL && base->kind == EL_HIR_TYPE_STRUCT) {
                el_diag_help(
                    binder->diag, "got reference type '${type}', did you mean to dereference it?",
                    EL_DIAG_TYPE("type", type),
                );
            }
        }
        return NULL;
    }

    const ElHirStructType* stype = &type->as.struct_;

    bool found = false;
    usize field_index = _el_binder_find_field(member->name, stype, &found);

    if (!found) {
        return el_diag_report(
            binder->diag, EL_DIAG_ERROR, "sema.unknown-field",
            in->span, "struct has no field named '${name}'",
            EL_DIAG_STRING("name", member->name)
        );
    }

    if (member->is_optional) {
        ElHirType* field_type = stype->fields[field_index].type;

        // expr ?> expr!.field
        ElHirType* result_type = el_hir_new_opt_type(binder->arena, field_type);
        return el_hir_new_bin_expr(binder->arena, in->span, result_type, EL_SEMA_BIN_OP_OPT_MAP, expr,
                el_hir_new_member_expr(
                    binder->arena, in->span, field_type, el_hir_new_unary_expr(
                        binder->arena, in->span, expr->type->as.opt.base, EL_SEMA_UNARY_OP_OPT_UNWRAP, expr),
            member->name, field_index
        ));
    }

    return el_hir_new_member_expr(
        binder->arena, in->span, stype->fields[field_index].type,
        expr, member->name, field_index
    );
}

ElHirExpr* _el_binder_bind_tmember_expr(ElBinder* binder, ElAstExpr* in, ElAstTMemberExpr* tmember) {
    (void) in;
    ElHirExpr* expr = el_binder_bind_expr(binder, tmember->expr);
    if (expr == NULL) return NULL;

    ElHirType* type = expr->type;
    if (type != NULL) type = el_hir_type_unwrap_distinct(type);

    if (tmember->is_optional) {
        if (type == NULL || type->kind != EL_HIR_TYPE_OPT) {
            return el_diag_report(
                binder->diag, EL_DIAG_ERROR, "sema.type-mismatch",
                tmember->expr->span, "optional member access requires an optional operand"
            );
        }
        type = el_hir_type_unwrap_distinct(type->as.opt.base);
    }

    if (type == NULL || type->kind != EL_HIR_TYPE_TUPLE) {
        el_diag_report(
            binder->diag, EL_DIAG_ERROR, "sema.not-tuple",
            tmember->expr->span, "tuple element access requires a tuple value",
        );
        if (type != NULL && type->kind == EL_HIR_TYPE_REF) {
            const ElHirType* base = el_hir_type_unwrap_distinct(type->as.ref.base);
            if (base != NULL && base->kind == EL_HIR_TYPE_TUPLE) {
                el_diag_help(
                    binder->diag, "got reference type '${type}', did you mean to dereference it?",
                    EL_DIAG_TYPE("type", type),
                );
            }
        }
        return NULL;
    }

    ElHirTupleType* ttype = &type->as.tuple;
    if (tmember->index >= ttype->count) {
        el_diag_report(
            binder->diag, EL_DIAG_ERROR, "sema.tuple-index-bounds",
            tmember->index_span, "out of bounds tuple element access",
        );
        el_diag_help(
            binder->diag, "expected value in range 0..${count}",
            EL_DIAG_INT("count", ttype->count),
        );
        return NULL;
    }

    if (tmember->is_optional) {
        ElHirType* elem_type   = ttype->elements[tmember->index];
        ElHirType* result_type = el_hir_new_opt_type(binder->arena, elem_type);

        // expr ?> expr!.index
        return el_hir_new_bin_expr(
            binder->arena, in->span, result_type, EL_SEMA_BIN_OP_OPT_MAP, expr,
                el_hir_new_tmember_expr(
                    binder->arena, in->span, elem_type, el_hir_new_unary_expr(
                        binder->arena, in->span, type, EL_SEMA_UNARY_OP_OPT_UNWRAP, expr),
                tmember->index
            )
        );
    }

    return el_hir_new_tmember_expr(
        binder->arena, in->span, ttype->elements[tmember->index],
        expr, tmember->index
    );
}

ElHirExpr* _el_binder_bind_expr_impl(ElBinder* binder, ElAstExpr* in) {
    switch (in->type) {
    case EL_AST_EXPR_BINARY:    return _el_binder_bind_bin_expr(binder, in, &in->as.binary);
    case EL_AST_EXPR_UNARY:     return _el_binder_bind_unary_expr(binder, in, &in->as.unary);
    case EL_AST_EXPR_LITERAL:   return _el_binder_bind_literal(binder, in, &in->as.literal);
    case EL_AST_EXPR_IDENT:     return _el_binder_bind_ident(binder, in, &in->as.ident);
    case EL_AST_EXPR_CALL:      return _el_binder_bind_call(binder, in, &in->as.call);
    case EL_AST_EXPR_CAST:      return _el_binder_bind_cast(binder, in, &in->as.cast);
    case EL_AST_EXPR_TYPEDINIT: return el_binder_bind_typedinit(binder, in, &in->as.typedinit);
    case EL_AST_EXPR_MEMBER:    return _el_binder_bind_member_expr(binder, in, &in->as.member);
    case EL_AST_EXPR_TMEMBER:   return _el_binder_bind_tmember_expr(binder, in, &in->as.tmember);
    }
    EL_UNREACHABLE_ENUM_VAL(ElAstExprType, in->type);
}

ElHirExpr* el_binder_bind_expr(ElBinder* binder, ElAstExpr* in) {
    EL_ASSERT(in != NULL, "should not be NULL");
    el_prof_begin_sub(binder->prof, binder->pss_expr);

    ElHirExpr* expr = _el_binder_bind_expr_impl(binder, in);
    if (expr == NULL) {
        el_prof_finish_sub(binder->prof, binder->pss_expr);
        return NULL;
    }

    el_prof_finish_sub(binder->prof, binder->pss_expr);
    return _el_binder_simplify_expr(binder, expr);
}
