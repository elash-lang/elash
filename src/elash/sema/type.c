#include <elash/sema/type.h>

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
}

