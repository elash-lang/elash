#include <elash/hir/type.h>
#include <elash/util/assert.h>
#include <elash/util/hash.h>

#include <stdint.h>

static void format_mut(ElMutabilitySpec mut, ElWriter out) {
    switch (mut) {
    case EL_MUTSPEC_DEFAULT: return;
    case EL_MUTSPEC_CONST:   el_swrite(out, "const "); return;
    case EL_MUTSPEC_WONLY:   el_swrite(out, "wonly "); return;
    }
    EL_UNREACHABLE_ENUM_VAL(ElMutabilitySpec, mut);
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity): the logic is flat
void el_hir_dump_type(const ElHirType* type, ElWriter out) {
    switch (type->kind) {
    case EL_HIR_TYPE_QUAL:
        format_mut(type->as.qual.mut, out);
        el_hir_dump_type(type->as.qual.base, out);
        return;
    case EL_HIR_TYPE_PRIM:
        switch (type->as.prim.kind) {
        case EL_HIR_PRIMTYPE_VOID: el_swrite(out, "void"); return;
        case EL_HIR_PRIMTYPE_BOOL: el_swrite(out, "bool"); return;
        case EL_HIR_PRIMTYPE_INT:
            el_write(out, ((ElStringView[][2]) {
                [EL_HIR_IWIDTH_NATIVE]    = { EL_SV("usize"),   EL_SV("isize")  },
                [EL_HIR_IWIDTH_EFFICIENT] = { EL_SV("uint"),    EL_SV("int")    },
                [EL_HIR_IWIDTH_8]         = { EL_SV("uint8"),   EL_SV("int8")   },
                [EL_HIR_IWIDTH_16]        = { EL_SV("uint16"),  EL_SV("int16")  },
                [EL_HIR_IWIDTH_32]        = { EL_SV("uint32"),  EL_SV("int32")  },
                [EL_HIR_IWIDTH_64]        = { EL_SV("uint64"),  EL_SV("int64")  },
                [EL_HIR_IWIDTH_128]       = { EL_SV("uint128"), EL_SV("int128") },
            }[type->as.prim.as.integral.width][type->as.prim.as.integral.is_signed]));
            return;
        case EL_HIR_PRIMTYPE_FLOAT:
            el_write(out, ((ElStringView[]) {
                [EL_HIR_FPWIDTH_EFFICIENT] = EL_SV("float"),
                [EL_HIR_FPWIDTH_16]        = EL_SV("float16"),
                [EL_HIR_FPWIDTH_32]        = EL_SV("float32"),
                [EL_HIR_FPWIDTH_64]        = EL_SV("float64"),
                [EL_HIR_FPWIDTH_128]       = EL_SV("float128"),
            }[type->as.prim.as.fp.width]));
            return;
        }
        EL_UNREACHABLE_ENUM_VAL(ElHirPrimTypeKind, type->as.prim.kind);
    case EL_HIR_TYPE_REF:
        el_hir_dump_type(type->as.ref.base, out);
        el_write_char(out, '&');
        return;
    case EL_HIR_TYPE_OPT:
        el_hir_dump_type(type->as.opt.base, out);
        el_write_char(out, '?');
        return;
    case EL_HIR_TYPE_DISTINCT: {
        ElStringView name = type->as.distinct.name;
        el_write(out, name);
        return;
    }
    case EL_HIR_TYPE_ARRAY:
        el_hir_dump_type(type->as.array.base, out);
        el_write_char(out, '[');
        el_writef(out, "%zu", type->as.array.size);
        el_write_char(out, ']');
        return;
    case EL_HIR_TYPE_SLICE:
        el_hir_dump_type(type->as.slice.base, out);
        el_swrite(out, "[]");
        return;
    case EL_HIR_TYPE_RWSLICE:
        el_hir_dump_type(type->as.rwslice.base, out);
        el_swrite(out, "[&]");
        return;
    case EL_HIR_TYPE_FUNC:
        el_hir_dump_type(type->as.func.ret_type, out);
        el_write_char(out, '(');
        for (usize i = 0; i < type->as.func.param_count; i++) {
            el_hir_dump_type(type->as.func.params[i], out);
            if (i + 1 < type->as.func.param_count) el_swrite(out, ", ");
        }
        el_write_char(out, ')');
        return;
    case EL_HIR_TYPE_STRUCT:
        el_swrite(out, "struct {");
        for (usize i = 0; i < type->as.struct_.count; i++) {
            ElStringView name = type->as.struct_.fields[i].name;
            el_write(out, name);
            el_swrite(out, ": ");
            el_hir_dump_type(type->as.struct_.fields[i].type, out);
            if (i + 1 < type->as.struct_.count) el_swrite(out, ", ");
        }
        el_write_char(out, '}');
        return;
    case EL_HIR_TYPE_TUPLE:
        el_swrite(out, "struct(");
        for (usize i = 0; i < type->as.tuple.count; i++) {
            el_hir_dump_type(type->as.tuple.elements[i], out);
            if (i + 1 < type->as.tuple.count) el_swrite(out, ", ");
        }
        el_write_char(out, ')');
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
        case EL_HIR_PRIMTYPE_INT:
            return lhs->as.prim.as.integral.width == rhs->as.prim.as.integral.width
                && lhs->as.prim.as.integral.is_signed == rhs->as.prim.as.integral.is_signed;
        case EL_HIR_PRIMTYPE_FLOAT:
            return lhs->as.prim.as.fp.width == rhs->as.prim.as.fp.width;
        case EL_HIR_PRIMTYPE_VOID:
        case EL_HIR_PRIMTYPE_BOOL:
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
        if (type->as.prim.kind == EL_HIR_PRIMTYPE_INT) {
            hash = el_hash_mix(hash, (uhash)type->as.prim.as.integral.width);
            hash = el_hash_mix(hash, (uhash)type->as.prim.as.integral.is_signed);
        } else if (type->as.prim.kind == EL_HIR_PRIMTYPE_FLOAT) {
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

    case EL_HIR_TYPE_STRUCT:
        EL_ASSERT(from->as.struct_.count != to->as.struct_.count,
                  "invalid arguments passed");

        for (usize i = 0; i < from->as.struct_.count; i++) {
            if (!el_hir_type_mut_compatible(from->as.struct_.fields[i].type, to->as.struct_.fields[i].type)) {
                return false;
            }
        }
        return true;

    case EL_HIR_TYPE_TUPLE:
        EL_ASSERT(from->as.tuple.count != to->as.tuple.count,
                  "invalid arguments passed");

        for (usize i = 0; i < from->as.tuple.count; i++) {
            if (!el_hir_type_mut_compatible(from->as.tuple.elements[i], to->as.tuple.elements[i])) {
                return false;
            }
        }
        return true;

    default:
        return true;
    }
}

static bool type_has_mut(const ElHirType* type, ElMutabilitySpec mut) {
    if (type == NULL) return false;
    if (el_hir_type_mut(type) == mut) return true;

    type = el_hir_type_unwrap(type);
    if (type == NULL) return false;

    switch (type->kind) {
    case EL_HIR_TYPE_ARRAY:
        return type_has_mut(type->as.array.base, mut);
    case EL_HIR_TYPE_TUPLE:
        for (usize i = 0; i < type->as.tuple.count; i++) {
            if (type_has_mut(type->as.tuple.elements[i], mut))
                return true;
        }
        return false;
    case EL_HIR_TYPE_STRUCT:
        for (usize i = 0; i < type->as.struct_.count; i++) {
            if (type_has_mut(type->as.struct_.fields[i].type, mut))
                return true;
        }
        return false;
    default:
        return false;
    }
}

bool el_hir_type_is_writable(const ElHirType* type) {
    return el_hir_type_mut(type) != EL_MUTSPEC_CONST;
}

bool el_hir_type_is_readable(const ElHirType* type) {
    return !type_has_mut(type, EL_MUTSPEC_WONLY);
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

const ElHirType* _el_hir_type_canonical_impl(const ElHirType* type) {
    while (type != NULL && type->kind == EL_HIR_TYPE_QUAL)
        type = type->as.qual.base;

    return type;
}

const ElHirType* _el_hir_type_unwrap_impl(const ElHirType* type) {
    type = _el_hir_type_canonical_impl(type);

    while (type != NULL && type->kind == EL_HIR_TYPE_DISTINCT) {
        if (type->as.distinct.orig == NULL)
            break;

        type = type->as.distinct.orig;
    }
    return type;
}

bool el_hir_type_is_incomplete(const ElHirType* type) {
    type = el_hir_type_unwrap(type);
    return type->kind == EL_HIR_TYPE_DISTINCT
        || type->kind == EL_HIR_TYPE_FUNC
        || (type->kind == EL_HIR_TYPE_PRIM && type->as.prim.kind == EL_HIR_PRIMTYPE_VOID);
}

bool el_hir_type_is_view(const ElHirType* type) {
    type = el_hir_type_canonical(type);
    return type != NULL
        && (type->kind == EL_HIR_TYPE_REF
         || type->kind == EL_HIR_TYPE_SLICE
         || type->kind == EL_HIR_TYPE_RWSLICE);
}
