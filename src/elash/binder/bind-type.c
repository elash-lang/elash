#include "binder-internals.h"

#include <elash/diag/engine.h>
#include <elash/util/assert.h>
#include <elash/util/todo.h>

#include <elash/hir/type.h>

static ElHirType* bind_array_type(ElBinder* binder, ElAstArrayType* array) {
    ElHirType* base = el_bind_type(binder, array->base);
    if (!_el_binder_ensure_complete(binder, array->base->span, base))
        return NULL;

    usize size;
    if (!_el_binder_eval_const_index(binder, array->size, &size))
        return NULL;

    return el_hir_new_array_type(binder->arena, base, size);
}

static usize count_struct_fields(ElAstStructType* struct_) {
    usize field_count = 0;
    for (ElAstDecl* field = struct_->fields; field != NULL; field = field->next) {
        if (field->type == EL_AST_DECL_VAR_DEF) {
            for (ElAstDeclarator* d = field->as.var_def.declarators; d != NULL; d = d->next) {
                field_count++;
            }
        } else if (field->type == EL_AST_DECL_VAR_DECL) {
            for (ElAstDeclarator* d = field->as.var_decl.declarators; d != NULL; d = d->next) {
                field_count++;
            }
        } else if (field->type == EL_AST_DECL_FUNC_DECL || field->type == EL_AST_DECL_FUNC_DEF) {
            field_count++;
        }
    }
    return field_count;
}

static ElHirType* bind_struct_type(ElBinder* binder, ElAstStructType* struct_) {
    usize field_capacity = count_struct_fields(struct_);
    ElHirStructField* fields = EL_DYNARENA_NEW_ARR(
        binder->arena, ElHirStructField, field_capacity);

    _el_binder_push_scope(binder);

    usize i = 0;
    for (ElAstDecl* field = struct_->fields; field != NULL; field = field->next) {
        switch (field->type) {
        case EL_AST_DECL_VAR_DEF: {
            ElHirType* type = el_bind_type(binder, field->as.var_def.type);
            for (ElAstDeclarator* d = field->as.var_def.declarators; d != NULL; d = d->next) {
                if (!_el_binder_ensure_complete(binder, field->span, type))
                    type = binder->builtins->type_void;

                fields[i++] = (ElHirStructField) {
                    .name = d->name->name,
                    .type = type,
                };
            }
            break;
        }
        case EL_AST_DECL_VAR_DECL: {
            el_diag_report(
                binder->diag, EL_DIAG_ERROR, "sema.bad-struct-field",
                field->span, "definition expected",
            );

            // just for better error reporting
            ElHirType* type = el_bind_type(binder, field->as.var_decl.type);
            for (ElAstDeclarator* d = field->as.var_decl.declarators; d != NULL; d = d->next) {
                fields[i++] = (ElHirStructField) {
                    .name = d->name->name,
                    .type = type ? type : binder->builtins->type_void,
                };
            }
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
            fields[i++] = (ElHirStructField) {
                .name = field->type == EL_AST_DECL_FUNC_DECL
                    ? field->as.func_decl.sig.name->name
                    : field->as.func_def.sig.name->name,
                .type = binder->builtins->type_void,
            };
            break;

        case EL_AST_DECL_ALIAS:
        case EL_AST_DECL_TYPEDEF:
            el_bind_decl(binder, field);
            break;
        }
    }

    _el_binder_pop_scope(binder);

    return el_hir_new_struct_type(binder->arena, fields, i);
}

static ElHirType* bind_tuple_type(ElBinder* binder, ElAstTupleType* tuple) {
    ElHirType** elements = EL_DYNARENA_NEW_ARR(
        binder->arena, ElHirType*, tuple->count);

    usize i = 0;
    for (ElAstType* type = tuple->head; type != NULL; ++i, type = type->next) {
        elements[i] = el_bind_type(binder, type);
        if (!_el_binder_ensure_complete(binder, type->span, elements[i])) {
            // again, for better error reporting
            elements[i] = binder->builtins->type_void;
        }
    }

    return el_hir_new_tuple_type(binder->arena, elements, tuple->count);
}

static ElHirType* _bind_type_internal(ElBinder* binder, ElAstType* in) {
    if (in->mut != EL_MUTSPEC_DEFAULT)
        EL_UNREACHABLE("mutability specifiers unimplemented");

    switch (in->kind) {
    case EL_AST_TYPE_REF: {
        ElHirType* base = el_bind_type(binder ,in->as.ref.base);
        if (base == NULL) return NULL;
        return el_hir_new_ref_type(binder->arena, base);
    }
    case EL_AST_TYPE_OPT: {
        ElHirType* base = el_bind_type(binder ,in->as.ref.base);
        if (base == NULL) return NULL;
        return el_hir_new_opt_type(binder->arena, base);
    }
    case EL_AST_TYPE_SLICE: {
        ElHirType* base = el_bind_type(binder, in->as.slice.base);
        if (base == NULL) return NULL;
        return (in->as.slice.is_raw ? el_hir_new_raw_slice_type : el_hir_new_slice_type)(binder->arena, base);
    }
    case EL_AST_TYPE_NAME: {
        ElHirSymbol* sym = el_hir_scope_lookup(binder->current_scope, in->as.name->name);
        if (sym == NULL) return NULL;

        if (sym->kind != EL_SYM_TYPE)
            return el_diag_report(
                binder->diag, EL_DIAG_ERROR, "sema.unexpected-symbol-kind",
                in->span, "${type} ${name} used as a type",
                EL_DIAG_STRING("type", sym->kind == EL_SYM_VAR ? EL_SV("variable") : EL_SV("function")),
                EL_DIAG_STRING("name", sym->name),
            );

        return sym->as.type.type;
    }
    case EL_AST_TYPE_STRUCT:
        return bind_struct_type(binder, &in->as.struct_);
    case EL_AST_TYPE_TUPLE:
        return bind_tuple_type(binder, &in->as.tuple);
    case EL_AST_TYPE_ARRAY:
        return bind_array_type(binder, &in->as.array);
    }

    EL_UNREACHABLE_ENUM_VAL(ElAstTypeKind, in->kind);
}


ElHirType* el_bind_type(ElBinder* binder, ElAstType* in) {
    EL_ASSERT(in != NULL, "should not be NULL");
    el_prof_begin_sub(binder->prof, binder->pss_type);
    ElHirType* result = _bind_type_internal(binder, in);
    el_prof_finish_sub(binder->prof, binder->pss_type);
    return result;
}
