#include <elash/hir/type.h>
#include <elash/util/assert.h>
#include <elash/util/hash.h>

#include <stdint.h>

static void stdio_wrapper(const char* pointer_to_const_char, void* pointer_to_void) {
    fputs(pointer_to_const_char, (FILE*) pointer_to_void);
}
static void strbuf_wrapper(const char* pointer_to_const_char, void* pointer_to_void) {
    el_strbuf_append_cstr((ElStringBuf*) pointer_to_void, pointer_to_const_char);
}

void el_hir_dump_type(const ElHirType* type, FILE* out) {
    el_format_type_internal(type, stdio_wrapper, out);
}
void el_format_type(const ElHirType* type, ElStringBuf* sb) {
    el_format_type_internal(type, strbuf_wrapper, sb);
}

static inline void writeint(usize tuff, void (*write)(const char*, void*), void* ctx) {
    char tuff_buff[31]; // NOLINT(readability-magic-numbers)
    snprintf(tuff_buff, sizeof tuff_buff, "%zu", tuff);
    write(tuff_buff, ctx);
}

static void format_mut(ElMutabilitySpec mut, void (*write)(const char*, void*), void* ctx) {
    switch (mut) {
    case EL_MUTSPEC_DEFAULT: return;
    case EL_MUTSPEC_CONST:   write("const ", ctx); return;
    case EL_MUTSPEC_WONLY:   write("wonly ", ctx); return;
    }
    EL_UNREACHABLE_ENUM_VAL(ElMutabilitySpec, mut);
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity): the logic is flat
void el_format_type_internal(const ElHirType* type, void (*write)(const char*, void*), void* ctx) {
    switch (type->kind) {
    case EL_HIR_TYPE_QUAL:
        format_mut(type->as.qual.mut, write, ctx);
        el_format_type_internal(type->as.qual.base, write, ctx);
        return;
    case EL_HIR_TYPE_PRIM:
        switch (type->as.prim.kind) {
        case EL_PRIMTYPE_VOID: write("void", ctx); return;
        case EL_PRIMTYPE_BOOL: write("bool", ctx); return;
        case EL_PRIMTYPE_INT:
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
        case EL_PRIMTYPE_FLOAT:
            write((const char* const[]) {
                [EL_HIR_FPWIDTH_EFFICIENT] = "float",
                [EL_HIR_FPWIDTH_16]        = "float16",
                [EL_HIR_FPWIDTH_32]        = "float32",
                [EL_HIR_FPWIDTH_64]        = "float64",
                [EL_HIR_FPWIDTH_128]       = "float128",
            }[type->as.prim.as.fp.width], ctx);
            return;
        }
        EL_UNREACHABLE_ENUM_VAL(ElHirPrimTypeKind, type->as.prim.kind);
    case EL_HIR_TYPE_REF:
        el_format_type_internal(type->as.ref.base, write, ctx);
        write("&", ctx);
        return;
    case EL_HIR_TYPE_OPT:
        el_format_type_internal(type->as.opt.base, write, ctx);
        write("?", ctx);
        return;
    case EL_HIR_TYPE_DISTINCT: {
        ElStringView name = type->as.distinct.name;
        char buf[2] = { '\0', '\0' };
        for (usize j = 0; j < name.len; j++) {
            buf[0] = name.data[j];
            write(buf, ctx);
        }
        return;
    }
    case EL_HIR_TYPE_ARRAY:
        el_format_type_internal(type->as.array.base, write, ctx);
        write("[", ctx);
        writeint(type->as.array.size, write, ctx);
        write("]", ctx);
        return;
    case EL_HIR_TYPE_SLICE:
        el_format_type_internal(type->as.slice.base, write, ctx);
        write("[]", ctx);
        return;
    case EL_HIR_TYPE_RWSLICE:
        el_format_type_internal(type->as.rwslice.base, write, ctx);
        write("[&]", ctx);
        return;
    case EL_HIR_TYPE_FUNC:
        el_format_type_internal(type->as.func.ret_type, write, ctx);
        write("(", ctx);
        for (usize i = 0; i < type->as.func.param_count; i++) {
            el_format_type_internal(type->as.func.params[i], write, ctx);
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
            el_format_type_internal(type->as.struct_.fields[i].type, write, ctx);
            if (i + 1 < type->as.struct_.count) write(", ", ctx);
        }
        write("}", ctx);
        return;
    case EL_HIR_TYPE_TUPLE:
        write("(", ctx);
        for (usize i = 0; i < type->as.tuple.count; i++) {
            el_format_type_internal(type->as.tuple.elements[i], write, ctx);
            if (i + 1 < type->as.tuple.count) write(", ", ctx);
        }
        write(")", ctx);
        return;
    }
    EL_UNREACHABLE_ENUM_VAL(ElHirTypeKind, type->kind);
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity): the logic is flat
static bool el_hir_type_eql_impl(const ElHirType* lhs, const ElHirType* rhs, bool unqual) {
    if (lhs == NULL || rhs == NULL) return lhs == rhs;

    if (unqual) {
        lhs = el_hir_type_canonical((ElHirType*)lhs);
        rhs = el_hir_type_canonical((ElHirType*)rhs);
        if (lhs == NULL || rhs == NULL) return lhs == rhs;
    }

    if (lhs == rhs)             return true;
    if (lhs->kind != rhs->kind) return false;

    switch (lhs->kind) {
    case EL_HIR_TYPE_QUAL:
        EL_ASSERT(!unqual, "Qual(Qual(T, ...), ...) is not allowed");
        return lhs->as.qual.mut == rhs->as.qual.mut
            && el_hir_type_eql_impl(lhs->as.qual.base, rhs->as.qual.base, false);

    case EL_HIR_TYPE_DISTINCT:
        return false; // if its the same type it should be caught by pointer
                      // comparison 9 lines above.

    case EL_HIR_TYPE_PRIM:
        if (lhs->as.prim.kind != rhs->as.prim.kind) return false;
        switch (lhs->as.prim.kind) {
        case EL_PRIMTYPE_INT:
            return lhs->as.prim.as.integral.width == rhs->as.prim.as.integral.width
                && lhs->as.prim.as.integral.is_signed == rhs->as.prim.as.integral.is_signed;
        case EL_PRIMTYPE_FLOAT:
            return lhs->as.prim.as.fp.width == rhs->as.prim.as.fp.width;
        case EL_PRIMTYPE_VOID:
        case EL_PRIMTYPE_BOOL:
            return true;
        }
        EL_UNREACHABLE_ENUM_VAL(ElHirPrimTypeKind, lhs->as.prim.kind);

    case EL_HIR_TYPE_REF:
        return el_hir_type_eql_impl(lhs->as.ref.base, rhs->as.ref.base, unqual);
    case EL_HIR_TYPE_OPT:
        return el_hir_type_eql_impl(lhs->as.opt.base, rhs->as.opt.base, unqual);
    case EL_HIR_TYPE_SLICE:
        return el_hir_type_eql_impl(lhs->as.slice.base, rhs->as.slice.base, unqual);
    case EL_HIR_TYPE_RWSLICE:
        return el_hir_type_eql_impl(lhs->as.rwslice.base, rhs->as.rwslice.base, unqual);

    case EL_HIR_TYPE_ARRAY:
        return lhs->as.array.size == rhs->as.array.size
            && el_hir_type_eql_impl(lhs->as.array.base, rhs->as.array.base, unqual);

    case EL_HIR_TYPE_FUNC:
        if (lhs->as.func.param_count != rhs->as.func.param_count) return false;
        if (!el_hir_type_eql_impl(lhs->as.func.ret_type, rhs->as.func.ret_type, unqual)) return false;
        for (usize i = 0; i < lhs->as.func.param_count; ++i) {
            if (!el_hir_type_eql_impl(lhs->as.func.params[i], rhs->as.func.params[i], unqual)) {
                return false;
            }
        }
        return true;

    case EL_HIR_TYPE_STRUCT:
        if (lhs->as.struct_.count != rhs->as.struct_.count) return false;
        for (usize i = 0; i < lhs->as.struct_.count; i++) {
            if (!el_sv_eql(lhs->as.struct_.fields[i].name, rhs->as.struct_.fields[i].name)) return false;
            if (!el_hir_type_eql_impl(lhs->as.struct_.fields[i].type, rhs->as.struct_.fields[i].type, unqual)) return false;
        }
        return true;

    case EL_HIR_TYPE_TUPLE:
        if (lhs->as.tuple.count != rhs->as.tuple.count) return false;
        for (usize i = 0; i < lhs->as.tuple.count; i++) {
            if (!el_hir_type_eql_impl(lhs->as.tuple.elements[i], rhs->as.tuple.elements[i], unqual)) return false;
        }
        return true;
    }

    EL_UNREACHABLE_ENUM_VAL(ElHirTypeKind, lhs->kind);
}

bool el_hir_type_eql(const ElHirType* lhs, const ElHirType* rhs) {
    return el_hir_type_eql_impl(lhs, rhs, false);
}
bool el_hir_type_eql_unqual(const ElHirType* lhs, const ElHirType* rhs) {
    return el_hir_type_eql_impl(lhs, rhs, true);
}

uhash el_hir_type_hash(const ElHirType* type) {
    EL_ASSERT(type != NULL, "invalid argument");

    uhash hash = (uhash)type->kind;
    switch (type->kind) {
    case EL_HIR_TYPE_QUAL:
        hash = el_hash_mix(hash, (uhash)type->as.qual.mut);
        hash = el_hash_mix(hash, el_hir_type_hash(type->as.qual.base));
        break;
    case EL_HIR_TYPE_PRIM:
        hash = el_hash_mix(hash, (uhash)type->as.prim.kind);
        if (type->as.prim.kind == EL_PRIMTYPE_INT) {
            hash = el_hash_mix(hash, (uhash)type->as.prim.as.integral.width);
            hash = el_hash_mix(hash, (uhash)type->as.prim.as.integral.is_signed);
        } else if (type->as.prim.kind == EL_PRIMTYPE_FLOAT) {
            hash = el_hash_mix(hash, (uhash)type->as.prim.as.fp.width);
        }
        break;
    case EL_HIR_TYPE_REF:
        hash = el_hash_mix(hash, el_hir_type_hash(type->as.ref.base));
        break;
    case EL_HIR_TYPE_OPT:
        hash = el_hash_mix(hash, el_hir_type_hash(type->as.opt.base));
        break;
    case EL_HIR_TYPE_SLICE:
        hash = el_hash_mix(hash, el_hir_type_hash(type->as.slice.base));
        break;
    case EL_HIR_TYPE_RWSLICE:
        hash = el_hash_mix(hash, el_hir_type_hash(type->as.rwslice.base));
        break;
    case EL_HIR_TYPE_ARRAY:
        hash = el_hash_mix(hash, (uhash)type->as.array.size);
        hash = el_hash_mix(hash, el_hir_type_hash(type->as.array.base));
        break;
    case EL_HIR_TYPE_FUNC:
        hash = el_hash_mix(hash, (uhash)type->as.func.param_count);
        hash = el_hash_mix(hash, el_hir_type_hash(type->as.func.ret_type));
        for (usize i = 0; i < type->as.func.param_count; i++)
            hash = el_hash_mix(hash, el_hir_type_hash(type->as.func.params[i]));
        break;
    case EL_HIR_TYPE_STRUCT:
        hash = el_hash_mix(hash, (uhash)type->as.struct_.count);
        for (usize i = 0; i < type->as.struct_.count; i++) {
            hash = el_hash_mix(hash, el_hash_string(type->as.struct_.fields[i].name));
            hash = el_hash_mix(hash, el_hir_type_hash(type->as.struct_.fields[i].type));
        }
        break;
    case EL_HIR_TYPE_TUPLE:
        hash = el_hash_mix(hash, (uhash)type->as.tuple.count);
        for (usize i = 0; i < type->as.tuple.count; i++)
            hash = el_hash_mix(hash, el_hir_type_hash(type->as.tuple.elements[i]));
        break;
    case EL_HIR_TYPE_DISTINCT:
        hash = el_hash_mix(hash, (uhash)(uintptr_t)type);
        break;
    }
    return hash;
}

ElMutabilitySpec el_hir_type_mut(const ElHirType* type) {
    if (type != NULL && type->kind == EL_HIR_TYPE_QUAL)
        return type->as.qual.mut;
    return EL_MUTSPEC_DEFAULT;
}

bool el_hir_type_mut_compatible(const ElHirType* from, const ElHirType* to) {
    ElMutabilitySpec fm = el_hir_type_mut(from);
    ElMutabilitySpec tm = el_hir_type_mut(to);

    // tm must be a subset of fm
    if (fm == EL_MUTSPEC_WONLY && tm != EL_MUTSPEC_WONLY)
        return false;
    if (tm == EL_MUTSPEC_WONLY && fm != EL_MUTSPEC_WONLY && fm != EL_MUTSPEC_DEFAULT)
        return false;
    if (fm == EL_MUTSPEC_CONST && tm != EL_MUTSPEC_CONST)
        return false;

    from = el_hir_type_canonical((ElHirType*)from);
    to   = el_hir_type_canonical((ElHirType*)to);
    if (from == NULL || to == NULL || from->kind != to->kind)
        return true;

    switch (from->kind) {
    case EL_HIR_TYPE_REF:     return el_hir_type_mut_compatible(from->as.ref.base, to->as.ref.base);
    case EL_HIR_TYPE_OPT:     return el_hir_type_mut_compatible(from->as.opt.base, to->as.opt.base);
    case EL_HIR_TYPE_SLICE:   return el_hir_type_mut_compatible(from->as.slice.base, to->as.slice.base);
    case EL_HIR_TYPE_RWSLICE: return el_hir_type_mut_compatible(from->as.rwslice.base, to->as.rwslice.base);
    case EL_HIR_TYPE_ARRAY:   return el_hir_type_mut_compatible(from->as.array.base, to->as.array.base);
    default:                  return true;
    }
}

bool el_hir_type_compatible(const ElHirType* from, const ElHirType* to) {
    return el_hir_type_eql_unqual(from, to)
        && el_hir_type_mut_compatible(from, to);
}

ElHirType* el_hir_type_qualify(ElDynArena* arena, ElHirType* type, ElMutabilitySpec mut) {
    if (type == NULL || mut == EL_MUTSPEC_DEFAULT)
        return type;
    if (type->kind == EL_HIR_TYPE_QUAL && type->as.qual.mut == mut)
        return type;
    return el_hir_new_qual_type(arena, el_hir_type_canonical(type), mut);
}

ElHirType* el_hir_type_canonical(ElHirType* type) {
    while (type != NULL && type->kind == EL_HIR_TYPE_QUAL)
        type = type->as.qual.base;

    return type;
}

ElHirType* el_hir_type_unwrap(ElHirType* type) {
    type = el_hir_type_canonical(type);

    while (type != NULL && type->kind == EL_HIR_TYPE_DISTINCT) {
        if (type->as.distinct.orig == NULL)
            break;

        type = type->as.distinct.orig;
    }
    return type;
}

bool el_hir_type_is_incomplete(const ElHirType* type) {
    type = el_hir_type_unwrap((ElHirType*)type);
    return type->kind == EL_HIR_TYPE_DISTINCT
        || type->kind == EL_HIR_TYPE_FUNC
        || (type->kind == EL_HIR_TYPE_PRIM && type->as.prim.kind == EL_PRIMTYPE_VOID);
}
