#include <elash/binder/binder.h>
#include <elash/util/assert.h>
#include <elash/util/todo.h>

#include <elash/diag/engine.h>
#include <elash/diag/meta.h>

#include <elash/hir/type.h>

ElHirType* _el_binder_bind_array_type(ElBinder* binder, ElAstArrayType* array) {
    ElHirType* base = _el_binder_bind_type(binder, array->base);
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

    return el_hir_new_array_type(binder->type_arena, base, (usize)size_val);
}

ElHirType* _el_binder_bind_type(ElBinder* binder, ElAstType* type) {
    switch (type->kind) {
    case EL_AST_TYPE_REF: {
        ElHirType* base = _el_binder_bind_type(binder ,type->as.ref.base);
        if (base == NULL) return NULL;
        return el_hir_new_ref_type(binder->type_arena, base);
    }
    case EL_AST_TYPE_SLICE: {
        ElHirType* base = _el_binder_bind_type(binder, type->as.slice.base);
        if (base == NULL) return NULL;
        return (type->as.slice.is_raw ? el_hir_new_raw_slice_type : el_hir_new_slice_type)(binder->type_arena, base);
    }
    case EL_AST_TYPE_NAME: {
        ElHirSymbol* sym = el_hir_scope_lookup(binder->current_scope, type->as.name->name);
        if (sym == NULL) return NULL;

        if (sym->kind != EL_SYM_TYPE)
            return el_diag_report(
                binder->diag, EL_DIAG_ERROR, "sema.unexpected-symbol-kind",
                type->span, "${type} ${name} used as a type",
                EL_DIAG_STRING("type", sym->kind == EL_SYM_VAR ? EL_SV("variable") : EL_SV("function")),
                EL_DIAG_STRING("name", sym->name),
            );

        return sym->as.type.type;
    }
    case EL_AST_TYPE_ARRAY:
        return _el_binder_bind_array_type(binder, &type->as.array);
    }

    EL_UNREACHABLE_ENUM_VAL(ElAstTypeKind, type->kind);
}
