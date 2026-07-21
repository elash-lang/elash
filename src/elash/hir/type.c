#include <elash/hir/type.h>
#include <elash/util/assert.h>

static void stdio_wrapper(const char* pointer_to_const_char, void* pointer_to_void) {
    fputs(pointer_to_const_char, (FILE*) pointer_to_void);
}
static void strbuf_wrapper(const char* pointer_to_const_char, void* pointer_to_void) {
    el_strbuf_append_cstr((ElStringBuf*) pointer_to_void, pointer_to_const_char);
}

void el_sema_dump_type(const ElHirType* type, FILE* out) {
    el_sema_format_type_internal(type, stdio_wrapper, out);
}
void el_sema_format_type(const ElHirType* type, ElStringBuf* sb) {
    el_sema_format_type_internal(type, strbuf_wrapper, sb);
}

static inline void writeint(usize tuff, void (*write)(const char*, void*), void* ctx) {
    char tuff_buff[31]; // NOLINT(readability-magic-numbers)
    snprintf(tuff_buff, sizeof tuff_buff, "%zu", tuff);
    write(tuff_buff, ctx);
}

void el_sema_format_type_internal(const ElHirType* type, void (*write)(const char*, void*), void* ctx) {
    switch (type->kind) {
    case EL_HIR_TYPE_PRIM:
        switch (type->as.prim.kind) {
        case EL_PRIMTYPE_VOID: write("void", ctx); return;
        case EL_PRIMTYPE_CHAR: write("char", ctx); return;
        case EL_PRIMTYPE_BOOL: write("bool", ctx); return;

        case EL_PRIMTYPE_INT: {
            write((const char* const [][2]) {
                [EL_HIR_IWIDTH_NATIVE]    = { "usize",   "isize"  },
                [EL_HIR_IWIDTH_EFFICIENT] = { "uint",    "int"    },
                [EL_HIR_IWIDTH_8]         = { "uint8",   "int8"   },
                [EL_HIR_IWIDTH_16]        = { "uint16",  "int16"  },
                [EL_HIR_IWIDTH_32]        = { "uint32",  "int32"  },
                [EL_HIR_IWIDTH_64]        = { "uint64",  "int64"  },
                [EL_HIR_IWIDTH_128]       = { "uint128", "int128" },
            }[type->as.prim.as.integral.width][type->as.prim.as.integral.is_signed], ctx);
            return;
        }
        }
        EL_UNREACHABLE_ENUM_VAL(ElHirPrimTypeKind, type->as.prim.kind);
    case EL_HIR_TYPE_REF:
        el_sema_format_type_internal(type->as.ref.base, write, ctx);
        write("*", ctx);
        return;
    case EL_HIR_TYPE_ARRAY:
        el_sema_format_type_internal(type->as.array.base, write, ctx);
        write("[", ctx);
        writeint(type->as.array.size, write, ctx);
        write("]", ctx);
        return;
    case EL_HIR_TYPE_SLICE:
        el_sema_format_type_internal(type->as.slice.base, write, ctx);
        write("[]", ctx);
        return;
    case EL_HIR_TYPE_RWSLICE:
        el_sema_format_type_internal(type->as.rwslice.base, write, ctx);
        write("[*]", ctx);
        return;
    case EL_HIR_TYPE_FUNC:
        el_sema_format_type_internal(type->as.func.ret_type, write, ctx);
        write("(", ctx);
        for (usize i = 0; i < type->as.func.param_count; i++) {
            el_sema_format_type_internal(type->as.func.params[i], write, ctx);
            if (i + 1 < type->as.func.param_count) write(", ", ctx);
        }
        write(")", ctx);
        return;
    case EL_HIR_TYPE_STRUCT:
        write("struct {", ctx);
        for (usize i = 0; i < type->as.struct_.count; i++) {
            // TODO: this SUCKS. write probably should accept a string view.
            ElStringView name = type->as.struct_.fields[i].name;
            char buf[2] = { '\0', '\0' };
            for (usize j = 0; j < name.len; j++) {
                buf[0] = name.data[j];
                write(buf, ctx);
            }
            write(": ", ctx);
            el_sema_format_type_internal(type->as.struct_.fields[i].type, write, ctx);
            if (i + 1 < type->as.struct_.count) write(", ", ctx);
        }
        write("}", ctx);
        return;
    case EL_HIR_TYPE_TUPLE:
        write("(", ctx);
        for (usize i = 0; i < type->as.tuple.count; i++) {
            el_sema_format_type_internal(type->as.tuple.elements[i], write, ctx);
            if (i + 1 < type->as.tuple.count) write(", ", ctx);
        }
        write(")", ctx);
        return;
    }
    EL_UNREACHABLE_ENUM_VAL(ElHirTypeKind, type->kind);
}

bool el_hir_type_eql(const ElHirType* lhs, const ElHirType* rhs) {
    if (lhs == NULL || rhs == NULL) return lhs == rhs;
    if (lhs == rhs)                 return true;
    if (lhs->kind != rhs->kind)     return false;

    switch (rhs->kind) {
    case EL_HIR_TYPE_PRIM:
        if (lhs->as.prim.kind != rhs->as.prim.kind) {
            return false;
        }
        if (lhs->as.prim.kind == EL_PRIMTYPE_INT) {
            return lhs->as.prim.as.integral.width == rhs->as.prim.as.integral.width &&
                   lhs->as.prim.as.integral.is_signed == rhs->as.prim.as.integral.is_signed;
        }
        return true;
    case EL_HIR_TYPE_REF:
        return el_hir_type_eql(lhs->as.ref.base, rhs->as.ref.base);
    case EL_HIR_TYPE_SLICE:
        return el_hir_type_eql(lhs->as.slice.base, rhs->as.slice.base);
    case EL_HIR_TYPE_RWSLICE:
        return el_hir_type_eql(lhs->as.rwslice.base, rhs->as.rwslice.base);
    case EL_HIR_TYPE_ARRAY:
        return lhs->as.array.size == rhs->as.array.size &&
            el_hir_type_eql(lhs->as.array.base, rhs->as.array.base);
    case EL_HIR_TYPE_FUNC:
        if (lhs->as.func.param_count != rhs->as.func.param_count) {
            return false;
        }
        if (!el_hir_type_eql(lhs->as.func.ret_type, rhs->as.func.ret_type)) {
            return false;
        }
        for (usize i = 0; i < lhs->as.func.param_count; ++i) {
            if (!el_hir_type_eql(lhs->as.func.params[i], rhs->as.func.params[i])) {
                return false;
            }
        }
        return true;
    case EL_HIR_TYPE_STRUCT:
        if (lhs->as.struct_.count != rhs->as.struct_.count) return false;
        for (usize i = 0; i < lhs->as.struct_.count; i++) {
            if (!el_sv_eql(lhs->as.struct_.fields[i].name, rhs->as.struct_.fields[i].name)) return false;
            if (!el_hir_type_eql(lhs->as.struct_.fields[i].type, rhs->as.struct_.fields[i].type)) return false;
        }
        return true;
    case EL_HIR_TYPE_TUPLE:
        if (lhs->as.tuple.count != rhs->as.tuple.count) return false;
        for (usize i = 0; i < lhs->as.tuple.count; i++) {
            if (!el_hir_type_eql(lhs->as.tuple.elements[i], rhs->as.tuple.elements[i])) return false;
        }
        return true;
    }
    EL_UNREACHABLE_ENUM_VAL(ElHirTypeKind, lhs->kind);
}
