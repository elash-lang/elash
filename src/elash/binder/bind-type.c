#include <elash/binder/binder.h>
#include <elash/util/assert.h>
#include <elash/util/todo.h>

#include <elash/diag/engine.h>
#include <elash/diag/meta.h>

#include <elash/sema/type.h>

ElType* _el_binder_bind_array_type(ElBinder* binder, ElAstArrayType* array) {
    ElType* base = _el_binder_bind_type(binder, array->base);
    if (base == NULL) return NULL;

    ElHirExprNode* size_hir = el_binder_bind_expr(binder, array->size);
    if (size_hir == NULL) return NULL;

    if (size_hir->kind != EL_HIR_EXPR_LITERAL || size_hir->type->kind != EL_TYPE_PRIM || size_hir->type->as.prim.kind != EL_PRIMTYPE_INT) {
        EL_TODO("implement constant expressions evaluation");
    }

    int64_t size_val = size_hir->as.literal.as.int_;
    if (size_val <= 0) {
        el_diag_report(
            binder->diag, EL_DIAG_ERROR, "sema.invalid-array-size",
            array->size->span,
            "array size must be positive"
        );
        return NULL;
    }

    return el_sema_new_array_type(binder->type_arena, base, (usize)size_val);
}

ElType* _el_binder_bind_type(ElBinder* binder, ElAstTypeNode* node) {
    switch (node->kind) {
    case EL_AST_TYPE_PTR: {
        ElType* base = _el_binder_bind_type(binder ,node->base);
        if (base == NULL) return NULL;
        return el_sema_new_ptr_type(binder->type_arena, base);
    }
    case EL_AST_TYPE_SLICE: {
        ElType* base = _el_binder_bind_type(binder ,node->base);
        if (base == NULL) return NULL;
        return el_sema_new_slice_type(binder->type_arena, base);
    }
    case EL_AST_TYPE_RAW_SLICE: {
        ElType* base = _el_binder_bind_type(binder ,node->base);
        if (base == NULL) return NULL;
        return el_sema_new_raw_slice_type(binder->type_arena, base);
    }
    case EL_AST_TYPE_NAME: {
        ElSymbol* sym = el_sema_scope_lookup(binder->current_scope, node->name->name);
        if (sym == NULL) return NULL;

        if (sym->kind != EL_SYM_TYPE) {
            el_diag_report(
                binder->diag, EL_DIAG_ERROR, "sema.unexpected-symbol-kind",
                node->span,
                "${type} ${name} used as a type",
                EL_DIAG_STRING("type", sym->kind == EL_SYM_VAR ? EL_SV("Variable") : EL_SV("Function")),
                EL_DIAG_STRING("name", sym->name),
            );
            return NULL;
        }

        return sym->as.type.type;
    }
    case EL_AST_TYPE_ARRAY:
        return _el_binder_bind_array_type(binder, &node->array);
    }

    EL_UNREACHABLE_ENUM_VAL(ElAstTypeKind, node->kind);
}
