#include <elash/binder/binder.h>
#include <elash/util/assert.h>
#include <elash/util/todo.h>

#include <elash/diag/engine.h>
#include <elash/diag/meta.h>

#include <elash/hir/type.h>

static ElHirType* bind_array_type(ElBinder* binder, ElAstArrayType* array) {
    ElHirType* base = _el_binder_bind_type(binder, array->base);
    if (base == NULL) return NULL;

    ElHirExpr* size_hir = el_binder_bind_expr(binder, array->size);
    if (size_hir == NULL) return NULL;

    ElHirExpr* actual_size_hir = _el_binder_implicit_cast(binder, array->size->span, size_hir, binder->builtins->type_usize);
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

static ElHirType* bind_struct_type(ElBinder* binder, ElAstStructType* struct_) {
    ElHirStructField* fields = EL_DYNARENA_NEW_ARR(
        binder->hir_arena, ElHirStructField, struct_->count);

    _el_binder_push_scope(binder);

    usize i = 0;
    for (ElAstDecl* field = struct_->fields; field != NULL; ++i, field = field->next) {
        switch (field->type) {
        case EL_AST_DECL_VAR_DEF: {
            ElHirType* type = _el_binder_bind_type(binder, field->as.var_def.type);
            fields[i] = (ElHirStructField) {
                .name = field->as.var_def.name->name,
                .type = type ? type : binder->builtins->type_void,
            };
            break;
        }
        case EL_AST_DECL_VAR_DECL: {
            el_diag_report(
                binder->diag, EL_DIAG_ERROR, "sema.bad-struct-field",
                field->span, "definition expected",
            );

            // just for better error reporting
            ElHirType* type = _el_binder_bind_type(binder, field->as.var_decl.type);
            fields[i] = (ElHirStructField) {
                .name = field->as.var_decl.name->name,
                .type = type ? type : binder->builtins->type_void,
            };
            break;
        }
        case EL_AST_DECL_FUNC_DECL:
        case EL_AST_DECL_FUNC_DEF:
            el_diag_report(
                binder->diag, EL_DIAG_ERROR, "sema.struct-field-func",
                field->type == EL_AST_DECL_FUNC_DECL
                    ? field->as.func_decl.sig.span
                    : field->as.func_def.sig.span,
                "struct field declared as function",
            );

            // just for better error reporting
            fields[i] = (ElHirStructField) {
                .name = field->type == EL_AST_DECL_FUNC_DECL
                    ? field->as.func_decl.sig.name->name
                    : field->as.func_def.sig.name->name,
                .type = binder->builtins->type_void,
            };
            break;

        case EL_AST_DECL_ALIAS:
            // very cool trick
            // the loop increments i every iteration but struct-local aliases
            // are not declaring any fields so we can just decrement i here
            i--;
            el_binder_bind_decl(binder, field);
            break;
        }
    }

    _el_binder_pop_scope(binder);

    return el_hir_new_struct_type(binder->type_arena, fields, i);
}

static ElHirType* bind_tuple_type(ElBinder* binder, ElAstTupleType* tuple) {
    ElHirType** elements = EL_DYNARENA_NEW_ARR(
        binder->type_arena, ElHirType*, tuple->count);

    usize i = 0;
    for (ElAstType* type = tuple->head; type != NULL; ++i, type = type->next) {
        elements[i] = _el_binder_bind_type(binder, type);
        if (elements[i] == NULL) {
            // again, for better error reporting
            elements[i] = binder->builtins->type_void;
        }
    }

    return el_hir_new_tuple_type(binder->type_arena, elements, tuple->count);
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
    case EL_AST_TYPE_STRUCT:
        return bind_struct_type(binder, &type->as.struct_);
    case EL_AST_TYPE_TUPLE:
        return bind_tuple_type(binder, &type->as.tuple);
    case EL_AST_TYPE_ARRAY:
        return bind_array_type(binder, &type->as.array);
    }

    EL_UNREACHABLE_ENUM_VAL(ElAstTypeKind, type->kind);
}
