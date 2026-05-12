#include <elash/sema/type.h>
#include <elash/util/assert.h>

void el_sema_dump_type(const ElType* type, FILE* out) {
    switch (type->kind) {
    case EL_TYPE_PRIM:
        switch (type->as.prim.kind) {
        case EL_PRIMTYPE_INT:  fputs("int", out);  break;
        case EL_PRIMTYPE_UINT: fputs("uint", out); break;
        case EL_PRIMTYPE_CHAR: fputs("char", out); break;
        case EL_PRIMTYPE_BOOL: fputs("bool", out); break;
        }
        break;
    case EL_TYPE_PTR:
        el_sema_dump_type(type->as.ptr.base, out);
        fputs("*", out);
        break;
    case EL_TYPE_FUNC:
        el_sema_dump_type(type->as.func.ret_type, out);
        fputs("(", out);
        for (usize i = 0; i < type->as.func.param_count; i++) {
            el_sema_dump_type(type->as.func.params[i], out);
            if (i + 1 < type->as.func.param_count) fputs(", ", out);
        }
        fputs(")", out);
        break;
    }
    EL_UNREACHABLE_ENUM_VAL(ElTypeKind, type->kind);
}

bool el_sema_type_eql(const ElType* lhs, const ElType* rhs) {
    if (lhs == rhs)             return true;
    if (lhs->kind != rhs->kind) return false;

    switch (rhs->kind) {
    case EL_TYPE_PRIM:
        return lhs->as.prim.kind == rhs->as.prim.kind;
    case EL_TYPE_PTR:
        return el_sema_type_eql(lhs->as.ptr.base, rhs->as.ptr.base);
    case EL_TYPE_FUNC:
        if (!el_sema_type_eql(lhs->as.func.ret_type, rhs->as.func.ret_type)) {
            return false;
        }
        for (usize i = 0; i < lhs->as.func.param_count; ++i) {
            if (!el_sema_type_eql(lhs->as.func.params[i], rhs->as.func.params[i])) {
                return false;
            }
        }
        return true;
    }
    EL_UNREACHABLE_ENUM_VAL(ElTypeKind, lhs->kind);
}
