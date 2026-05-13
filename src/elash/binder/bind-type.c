#include <elash/binder/binder.h>
#include <elash/util/assert.h>

#include <elash/diag/engine.h>
#include <elash/diag/meta.h>

#include <elash/sema/type.h>

ElType* _el_binder_bind_type(ElBinder* binder, ElAstTypeNode* node) {
    switch (node->kind) {
    case EL_AST_TYPE_PTR: {
        ElType* base = _el_binder_bind_type(binder ,node->base);
        if (base == NULL) return NULL;

        return el_sema_new_ptr_type(binder->type_arena, base);
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
    }

    EL_UNREACHABLE_ENUM_VAL(ElAstTypeKind, node->kind);
}
