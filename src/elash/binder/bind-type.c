#include <elash/binder/binder.h>
#include <elash/util/assert.h>
#include <elash/util/todo.h>

#include <elash/diag/engine.h>
#include <elash/diag/meta.h>

#include <elash/sema/type.h>

ElType* _el_binder_bind_array_type(ElBinder* binder, ElAstArrayType* array) {
    ElType* base = _el_binder_bind_type(binder, array->base);
    if (base == NULL) return NULL;

    ElHirExpr* size_hir = el_binder_bind_expr(binder, array->size);
    if (size_hir == NULL) return NULL;

    ElHirExpr* actual_size_hir = _el_binder_implicit_cast(binder, array->size->span, size_hir, binder->builtins->type_int);
    if (actual_size_hir == NULL) return NULL;

    int64_t size_val = actual_size_hir->as.constant.as.int_;
    if (size_val <= 0)
        return el_diag_report(
            binder->diag, EL_DIAG_ERROR, "sema.invalid-array-size",
            array->size->span,
            "array size must be positive"
        );

    return el_sema_new_array_type(binder->type_arena, base, (usize)size_val);
}

ElType* _el_binder_bind_type(ElBinder* binder, ElAstType* node) {
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

        if (sym->kind != EL_SYM_TYPE)
            return el_diag_report(
                binder->diag, EL_DIAG_ERROR, "sema.unexpected-symbol-kind",
                node->span, "${type} ${name} used as a type",
                EL_DIAG_STRING("type", sym->kind == EL_SYM_VAR ? EL_SV("variable") : EL_SV("function")),
                EL_DIAG_STRING("name", sym->name),
            );

        return sym->as.type.type;
    }
    case EL_AST_TYPE_ARRAY:
        return _el_binder_bind_array_type(binder, &node->array);
    }

    EL_UNREACHABLE_ENUM_VAL(ElAstTypeKind, node->kind);
}
