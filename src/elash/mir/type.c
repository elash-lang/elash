#include <elash/mir/type.h>
#include <elash/util/assert.h>

static void stdio_wrapper(const char* s, void* ctx) {
    fputs(s, (FILE*)ctx);
}
static void strbuf_wrapper(const char* s, void* ctx) {
    el_strbuf_append_cstr((ElStringBuf*)ctx, s);
}

void el_mir_dump_type(const ElMirType* type, FILE* out) {
    el_mir_format_type_internal(type, stdio_wrapper, out);
}
void el_mir_format_type(const ElMirType* type, ElStringBuf* sb) {
    el_mir_format_type_internal(type, strbuf_wrapper, sb);
}

static inline void writeint(usize val, void (*write)(const char*, void*), void* ctx) {
    char buf[31]; // NOLINT(readability-magic-numbers)
    snprintf(buf, sizeof buf, "%zu", val);
    write(buf, ctx);
}

void el_mir_format_type_internal(const ElMirType* type, void (*write)(const char*, void*), void* ctx) {
    switch (type->kind) {
    case EL_MIR_TYPE_VOID:
        write("void", ctx);
        return;
    case EL_MIR_TYPE_INT:
        if (type->as.integer.width == 1) {
            write("bool", ctx);
        } else {
            if (!type->as.integer.is_signed) {
                write("u", ctx);
            } else {
                write("i", ctx);
            }
            writeint(type->as.integer.width, write, ctx);
        }
        return;
    case EL_MIR_TYPE_PTR:
        if (type->as.ptr.base) {
            el_mir_format_type_internal(type->as.ptr.base, write, ctx);
        } else {
            write("void", ctx);
        }
        write("*", ctx);
        return;
    case EL_MIR_TYPE_ARRAY:
        if (type->as.array.base) {
            el_mir_format_type_internal(type->as.array.base, write, ctx);
        } else {
            write("void", ctx);
        }
        write("[", ctx);
        writeint(type->as.array.size, write, ctx);
        write("]", ctx);
        return;
    case EL_MIR_TYPE_FUNC:
        if (type->as.func.ret_type) {
            el_mir_format_type_internal(type->as.func.ret_type, write, ctx);
        } else {
            write("void", ctx);
        }
        write("(", ctx);
        for (usize i = 0; i < type->as.func.param_count; i++) {
            el_mir_format_type_internal(type->as.func.params[i], write, ctx);
            if (i + 1 < type->as.func.param_count) write(", ", ctx);
        }
        write(")", ctx);
        return;
    }
    EL_UNREACHABLE_ENUM_VAL(ElMirTypeKind, type->kind);
}

bool el_mir_type_eql(const ElMirType* lhs, const ElMirType* rhs) {
    if (lhs == NULL || rhs == NULL) return lhs == rhs;
    if (lhs == rhs)                 return true;
    if (lhs->kind != rhs->kind)     return false;

    switch (rhs->kind) {
    case EL_MIR_TYPE_VOID:
        return true;
    case EL_MIR_TYPE_INT:
        return lhs->as.integer.width == rhs->as.integer.width &&
               lhs->as.integer.is_signed == rhs->as.integer.is_signed;
    case EL_MIR_TYPE_PTR:
        return el_mir_type_eql(lhs->as.ptr.base, rhs->as.ptr.base);
    case EL_MIR_TYPE_ARRAY:
        return lhs->as.array.size == rhs->as.array.size &&
               el_mir_type_eql(lhs->as.array.base, rhs->as.array.base);
    case EL_MIR_TYPE_FUNC:
        if (lhs->as.func.param_count != rhs->as.func.param_count) {
            return false;
        }
        if (!el_mir_type_eql(lhs->as.func.ret_type, rhs->as.func.ret_type)) {
            return false;
        }
        for (usize i = 0; i < lhs->as.func.param_count; ++i) {
            if (!el_mir_type_eql(lhs->as.func.params[i], rhs->as.func.params[i])) {
                return false;
            }
        }
        return true;
    }
    EL_UNREACHABLE_ENUM_VAL(ElMirTypeKind, lhs->kind);
}
