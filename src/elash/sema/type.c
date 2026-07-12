#include <elash/sema/type.h>
#include <elash/util/assert.h>

void el_sema_dump_type(const ElType* type, FILE* out) {
    switch (type->kind) {
    case EL_TYPE_PRIM:
        switch (type->as.prim.kind) {
        case EL_PRIMTYPE_VOID: fputs("void", out); return;
        case EL_PRIMTYPE_CHAR: fputs("char", out); return;
        case EL_PRIMTYPE_BOOL: fputs("bool", out); return;

        case EL_PRIMTYPE_INT: {
            fputs((const char* const [][2]) {
                [EL_INT_WIDTH_NATIVE]    = { "usize",   "isize"  },
                [EL_INT_WIDTH_EFFICIENT] = { "uint",    "int"    },
                [EL_INT_WIDTH_8]         = { "uint8",   "int8"   },
                [EL_INT_WIDTH_16]        = { "uint16",  "int16"  },
                [EL_INT_WIDTH_32]        = { "uint32",  "int32"  },
                [EL_INT_WIDTH_64]        = { "uint64",  "int64"  },
                [EL_INT_WIDTH_128]       = { "uint128", "int128" },
            }[type->as.prim.as.integral.width][type->as.prim.as.integral.is_signed], out);
            return;
        }
        }
        EL_UNREACHABLE_ENUM_VAL(ElPrimitiveTypeKind, type->as.prim.kind);
    case EL_TYPE_PTR:
        el_sema_dump_type(type->as.ptr.base, out);
        fputs("*", out);
        return;
    case EL_TYPE_ARRAY:
        el_sema_dump_type(type->as.array.base, out);
        fprintf(out, "[%zu]", type->as.array.size);
        return;
    case EL_TYPE_SLICE:
        el_sema_dump_type(type->as.slice.base, out);
        fputs("[]", out);
        return;
    case EL_TYPE_RAW_SLICE:
        el_sema_dump_type(type->as.raw_slice.base, out);
        fputs("[*]", out);
        return;
    case EL_TYPE_FUNC:
        el_sema_dump_type(type->as.func.ret_type, out);
        fputs("(", out);
        for (usize i = 0; i < type->as.func.param_count; i++) {
            el_sema_dump_type(type->as.func.params[i], out);
            if (i + 1 < type->as.func.param_count) fputs(", ", out);
        }
        fputs(")", out);
        return;
    }
    EL_UNREACHABLE_ENUM_VAL(ElTypeKind, type->kind);
}

bool el_sema_type_eql(const ElType* lhs, const ElType* rhs) {
    if (lhs == rhs)             return true;
    if (lhs->kind != rhs->kind) return false;

    switch (rhs->kind) {
    case EL_TYPE_PRIM:
        if (lhs->as.prim.kind != rhs->as.prim.kind) {
            return false;
        }
        if (lhs->as.prim.kind == EL_PRIMTYPE_INT) {
            return lhs->as.prim.as.integral.width == rhs->as.prim.as.integral.width &&
                   lhs->as.prim.as.integral.is_signed == rhs->as.prim.as.integral.is_signed;
        }
        return true;
    case EL_TYPE_PTR:
        return el_sema_type_eql(lhs->as.ptr.base, rhs->as.ptr.base);
    case EL_TYPE_SLICE:
        return el_sema_type_eql(lhs->as.slice.base, rhs->as.slice.base);
    case EL_TYPE_RAW_SLICE:
        return el_sema_type_eql(lhs->as.raw_slice.base, rhs->as.raw_slice.base);
    case EL_TYPE_ARRAY:
        return lhs->as.array.size == rhs->as.array.size &&
            el_sema_type_eql(lhs->as.array.base, rhs->as.array.base);
    case EL_TYPE_FUNC:
        if (lhs->as.func.param_count != rhs->as.func.param_count) {
            return false;
        }
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
