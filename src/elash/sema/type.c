#include <elash/sema/type.h>
#include <elash/util/assert.h>

static void stdio_wrapper(const char* pointer_to_const_char, void* pointer_to_void) {
    fputs(pointer_to_const_char, (FILE*) pointer_to_void);
}
static void strbuf_wrapper(const char* pointer_to_const_char, void* pointer_to_void) {
    el_strbuf_append_cstr((ElStringBuf*) pointer_to_void, pointer_to_const_char);
}

void el_sema_dump_type(const ElType* type, FILE* out) {
    el_sema_format_type_internal(type, stdio_wrapper, out);
}
void el_sema_format_type(const ElType* type, ElStringBuf* sb) {
    el_sema_format_type_internal(type, strbuf_wrapper, sb);
}

static inline void writeint(usize tuff, void (*write)(const char*, void*), void* ctx) {
    char tuff_buff[31]; // NOLINT(readability-magic-numbers)
    snprintf(tuff_buff, sizeof tuff_buff, "%zu", tuff);
    write(tuff_buff, ctx);
}

void el_sema_format_type_internal(const ElType* type, void (*write)(const char*, void*), void* ctx) {
    switch (type->kind) {
    case EL_TYPE_PRIM:
        switch (type->as.prim.kind) {
        case EL_PRIMTYPE_VOID: write("void", ctx); return;
        case EL_PRIMTYPE_CHAR: write("char", ctx); return;
        case EL_PRIMTYPE_BOOL: write("bool", ctx); return;

        case EL_PRIMTYPE_INT: {
            write((const char* const [][2]) {
                [EL_INT_WIDTH_NATIVE]    = { "usize",   "isize"  },
                [EL_INT_WIDTH_EFFICIENT] = { "uint",    "int"    },
                [EL_INT_WIDTH_8]         = { "uint8",   "int8"   },
                [EL_INT_WIDTH_16]        = { "uint16",  "int16"  },
                [EL_INT_WIDTH_32]        = { "uint32",  "int32"  },
                [EL_INT_WIDTH_64]        = { "uint64",  "int64"  },
                [EL_INT_WIDTH_128]       = { "uint128", "int128" },
            }[type->as.prim.as.integral.width][type->as.prim.as.integral.is_signed], ctx);
            return;
        }
        }
        EL_UNREACHABLE_ENUM_VAL(ElPrimitiveTypeKind, type->as.prim.kind);
    case EL_TYPE_PTR:
        el_sema_format_type_internal(type->as.ptr.base, write, ctx);
        write("*", ctx);
        return;
    case EL_TYPE_ARRAY:
        el_sema_format_type_internal(type->as.array.base, write, ctx);
        write("[", ctx);
        writeint(type->as.array.size, write, ctx);
        write("]", ctx);
        return;
    case EL_TYPE_SLICE:
        el_sema_format_type_internal(type->as.slice.base, write, ctx);
        write("[]", ctx);
        return;
    case EL_TYPE_RAW_SLICE:
        el_sema_format_type_internal(type->as.raw_slice.base, write, ctx);
        write("[*]", ctx);
        return;
    case EL_TYPE_FUNC:
        el_sema_format_type_internal(type->as.func.ret_type, write, ctx);
        write("(", ctx);
        for (usize i = 0; i < type->as.func.param_count; i++) {
            el_sema_format_type_internal(type->as.func.params[i], write, ctx);
            if (i + 1 < type->as.func.param_count) write(", ", ctx);
        }
        write(")", ctx);
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
