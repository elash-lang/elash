#include <elash/binder/binder.h>
#include <elash/util/assert.h>

#include <elash/diag/engine.h>
#include <elash/diag/meta.h>

#include <elash/hir/tree/expr/array-lit.h>

ElHirExpr* _el_binder_bind_init_list(ElBinder* binder, ElAstInit* in, ElType* expected_type) {
    if (expected_type->kind != EL_TYPE_ARRAY) {
        el_diag_report(
            binder->diag, EL_DIAG_ERROR, "sema.init-non-aggregate",
            in->span,
            "initializer list can only be used with array and types"
        );
        return NULL;
    }

    ElType* base_type = expected_type->as.array.base;
    if (expected_type->kind == EL_TYPE_ARRAY && in->list.count != expected_type->as.array.size) {
        el_diag_report(
            binder->diag, EL_DIAG_ERROR, "sema.bad-init-list",
            in->span,
            "unexpected amount of elements in initializer list for array of size ${size}",
            EL_DIAG_INT("size", expected_type->as.array.size)
        );
        return NULL;
    }

    ElHirExpr** values = EL_DYNARENA_NEW_ARR(binder->hir_arena, ElHirExpr*, in->list.count);
    usize i = 0;
    for (ElAstInit* node = in->list.head; node != NULL; node = node->next, i++) {
        values[i] = el_binder_bind_init(binder, node, base_type);
        if (values[i] == NULL) return NULL;
    }

    return el_hir_new_array_lit(binder->hir_arena, expected_type, values, in->list.count);
}

ElHirExpr* el_binder_bind_init(ElBinder* binder, ElAstInit* in, ElType* expected_type) {
    switch (in->kind) {
    case EL_AST_INIT_EXPR: {
        ElHirExpr* expr = el_binder_bind_expr(binder, in->expr);
        if (expr == NULL) return NULL;

        if (!el_sema_type_eql(expr->type, expected_type)) {
            el_diag_report(
                binder->diag, EL_DIAG_ERROR, "sema.type-mismatch",
                in->span,
                "incompatible type in initializer"
            );
            return NULL;
        }
        return expr;
    }
    case EL_AST_INIT_LIST:
        return _el_binder_bind_init_list(binder, in, expected_type);
    }
    EL_UNREACHABLE_ENUM_VAL(ElAstInitKind, in->kind);
}
