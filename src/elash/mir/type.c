#include <elash/mir/type.h>
#include <elash/util/hash.h>
#include <elash/util/assert.h>

void el_mir_dump_type(const ElMirType* type, ElWriter* out) {
    switch (type->kind) {
    case EL_MIR_TYPE_VOID:
        el_swrite(out, "void");
        return;
    case EL_MIR_TYPE_INT:
        if (type->as.integer.width == 1) {
            el_swrite(out, "bool");
        } else {
            el_writef(out, "%s%zu",
                !type->as.integer.is_signed
                    ? "uint"
                    : "int",
                (usize)type->as.integer.width
            );
        }
        return;
    case EL_MIR_TYPE_FLOAT:
        el_writef(out, "float%zu", (usize)type->as.float_.width);
        return;
    case EL_MIR_TYPE_PTR:
        el_mir_dump_type(type->as.ptr.base, out);
        el_write_char(out, '*');
        return;
    case EL_MIR_TYPE_ARRAY:
        el_mir_dump_type(type->as.array.base, out);
        el_write_char(out, '[');
        el_writef(out, "%zu", type->as.array.size);
        el_write_char(out, ']');
        return;
    case EL_MIR_TYPE_TUPLE:
        el_swrite(out, "{ ");
        for (usize i = 0; i < type->as.tuple.item_count; ++i) {
            el_mir_dump_type(type->as.tuple.items[i], out);
            if (i + 1 < type->as.tuple.item_count) el_swrite(out, ", ");
        }
        el_swrite(out, " }");
        return;
    case EL_MIR_TYPE_FUNC:
        el_mir_dump_type(type->as.func.ret_type, out);
        el_write_char(out, '(');
        for (usize i = 0; i < type->as.func.param_count; i++) {
            el_mir_dump_type(type->as.func.params[i], out);
            if (i + 1 < type->as.func.param_count) el_swrite(out, ", ");
        }
        el_write_char(out, ')');
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
    case EL_MIR_TYPE_FLOAT:
        return lhs->as.float_.width == rhs->as.float_.width;
    case EL_MIR_TYPE_PTR:
        return el_mir_type_eql(lhs->as.ptr.base, rhs->as.ptr.base);
    case EL_MIR_TYPE_ARRAY:
        return lhs->as.array.size == rhs->as.array.size &&
               el_mir_type_eql(lhs->as.array.base, rhs->as.array.base);
    case EL_MIR_TYPE_TUPLE:
        if (lhs->as.tuple.item_count != rhs->as.tuple.item_count) {
            return false;
        }
        for (usize i = 0; i < lhs->as.tuple.item_count; ++i) {
            if (!el_mir_type_eql(lhs->as.tuple.items[i], rhs->as.tuple.items[i])) {
                return false;
            }
        }
        return true;
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

uhash el_mir_type_hash(const ElMirType* type) {
    EL_ASSERT(type != NULL, "invalid argument");

    uhash hash = (uhash)type->kind;
    switch (type->kind) {
    case EL_MIR_TYPE_INT:
        hash = el_hash_mix(hash, (uhash)type->as.integer.width);
        hash = el_hash_mix(hash, (uhash)type->as.integer.is_signed);
        break;
    case EL_MIR_TYPE_FLOAT:
        hash = el_hash_mix(hash, (uhash)type->as.float_.width);
        break;
    case EL_MIR_TYPE_PTR:
        hash = el_hash_mix(hash, el_mir_type_hash(type->as.ptr.base));
        break;
    case EL_MIR_TYPE_ARRAY:
        hash = el_hash_mix(hash, (uhash)type->as.array.size);
        hash = el_hash_mix(hash, el_mir_type_hash(type->as.array.base));
        break;
    case EL_MIR_TYPE_FUNC:
        hash = el_hash_mix(hash, (uhash)type->as.func.param_count);
        hash = el_hash_mix(hash, el_mir_type_hash(type->as.func.ret_type));
        for (usize i = 0; i < type->as.func.param_count; ++i)
            hash = el_hash_mix(hash, el_mir_type_hash(type->as.func.params[i]));
        break;
    case EL_MIR_TYPE_TUPLE:
        hash = el_hash_mix(hash, (uhash)type->as.tuple.item_count);
        for (usize i = 0; i < type->as.tuple.item_count; ++i)
            hash = el_hash_mix(hash, el_mir_type_hash(type->as.tuple.items[i]));
        break;
    case EL_MIR_TYPE_VOID:
        break;
    }
    return hash;
}
