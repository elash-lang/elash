#include "lowerer-internals.h"

#include <elash/util/assert.h>
#include <elash/util/todo.h>
#include <elash/mir/type.h>

static ElMirType* make_slice_type(ElTypeCache* tcache, const ElHirSliceType* slice) {
    ElMirType** items = EL_DYNARENA_NEW_ARR(tcache->arena, ElMirType*, 2);
    ElMirType*  base   = el_tcache_get_mir(tcache, slice->base);
    items[EL_MIR_SLICE_FIELD_DATA] = el_mir_new_ptr_type(tcache->arena, base);
    items[EL_MIR_SLICE_FIELD_LEN]  = tcache->usize_type;
    return el_mir_new_tuple_type(tcache->arena, items, 2);
}

static ElMirType* make_optional_type(ElTypeCache* tcache, const ElHirOptType* opt) {
    ElMirType** items = EL_DYNARENA_NEW_ARR(tcache->arena, ElMirType*, 2);
    ElMirType*  base   = el_tcache_get_mir(tcache, opt->base);
    items[OPT_FIELD_HAS_VALUE] = tcache->bool_type;
    items[OPT_FIELD_VALUE]     = base;
    return el_mir_new_tuple_type(tcache->arena, items, 2);
}

ElMirType* el_lowerer_map_type_raw(ElTypeCache* tcache, const ElHirType* type) {
    if (type == NULL) return NULL;
    switch (type->kind) {
    case EL_HIR_TYPE_PRIM:
        switch (type->as.prim.kind) {
        case EL_HIR_PRIMTYPE_VOID:
            return el_mir_new_void_type(tcache->arena);
        case EL_HIR_PRIMTYPE_BOOL:
            return el_mir_new_int_type(tcache->arena, 1, false);
        case EL_HIR_PRIMTYPE_INT: {
            uint32_t width = 0;
            // TODO: actually map native and efficient ints to correct width
            // NOLINTBEGIN(readability-magic-numbers)
            switch (type->as.prim.as.integral.width) {
            case EL_HIR_IWIDTH_NATIVE:    width = 64;  break;
            case EL_HIR_IWIDTH_EFFICIENT: width = 32;  break;
            case EL_HIR_IWIDTH_8:         width = 8;   break;
            case EL_HIR_IWIDTH_16:        width = 16;  break;
            case EL_HIR_IWIDTH_32:        width = 32;  break;
            case EL_HIR_IWIDTH_64:        width = 64;  break;
            case EL_HIR_IWIDTH_128:       width = 128; break;
            }
            // NOLINTEND(readability-magic-numbers)
            return el_mir_new_int_type(tcache->arena, width, type->as.prim.as.integral.is_signed);
        }
        case EL_HIR_PRIMTYPE_FLOAT: {
            uint32_t width = 0;
            // TODO: actually map efficient floats to correct width
            // NOLINTBEGIN(readability-magic-numbers)
            switch (type->as.prim.as.fp.width) {
            case EL_HIR_FPWIDTH_EFFICIENT: width = 32; break;
            case EL_HIR_FPWIDTH_16:        width = 16; break;
            case EL_HIR_FPWIDTH_32:        width = 32; break;
            case EL_HIR_FPWIDTH_64:        width = 64; break;
            case EL_HIR_FPWIDTH_128:       width = 128; break;
            }
            // NOLINTEND(readability-magic-numbers)
            return el_mir_new_float_type(tcache->arena, width);
        }
        }
        EL_UNREACHABLE("unknown prim type kind");
    case EL_HIR_TYPE_FUNC: {
        ElMirType* ret_type = el_tcache_get_mir(tcache, type->as.func.ret_type);
        ElMirType** params = EL_DYNARENA_NEW_ARR(tcache->arena, ElMirType*, type->as.func.param_count);
        for (usize i = 0; i < type->as.func.param_count; ++i) {
            params[i] = el_tcache_get_mir(tcache, type->as.func.params[i]);
        }
        return el_mir_new_func_type(tcache->arena, ret_type, params, type->as.func.param_count);
    }
    case EL_HIR_TYPE_ARRAY: {
        ElMirType* base = el_tcache_get_mir(tcache, type->as.array.base);
        return el_mir_new_array_type(tcache->arena, base, type->as.array.size);
    }
    case EL_HIR_TYPE_STRUCT: {
        ElMirType** elements = EL_DYNARENA_NEW_ARR(tcache->arena, ElMirType*, type->as.struct_.count);
        for (usize i = 0; i < type->as.struct_.count; ++i) {
            elements[i] = el_tcache_get_mir(tcache, type->as.struct_.fields[i].type);
        }
        return el_mir_new_tuple_type(tcache->arena, elements, type->as.struct_.count);
    }
    case EL_HIR_TYPE_TUPLE: {
        ElMirType** elements = EL_DYNARENA_NEW_ARR(tcache->arena, ElMirType*, type->as.tuple.count);
        for (usize i = 0; i < type->as.tuple.count; ++i) {
            elements[i] = el_tcache_get_mir(tcache, type->as.tuple.elements[i]);
        }
        return el_mir_new_tuple_type(tcache->arena, elements, type->as.tuple.count);
    }
    case EL_HIR_TYPE_REF:
        return el_mir_new_ptr_type(tcache->arena, el_tcache_get_mir(tcache, type->as.ref.base));
    case EL_HIR_TYPE_RWSLICE:
        return el_mir_new_ptr_type(tcache->arena, el_tcache_get_mir(tcache, type->as.rwslice.base));
    case EL_HIR_TYPE_SLICE:
        return make_slice_type(tcache, &type->as.slice);
    case EL_HIR_TYPE_OPT: {
        ElHirType* base = el_hir_type_canonical(type->as.opt.base);
        if (base->kind == EL_HIR_TYPE_REF)
            return el_mir_new_ptr_type(tcache->arena, el_tcache_get_mir(tcache, base->as.ref.base));
        if (base->kind == EL_HIR_TYPE_RWSLICE)
            return el_mir_new_ptr_type(tcache->arena, el_tcache_get_mir(tcache, base->as.rwslice.base));
        return make_optional_type(tcache, &type->as.opt);
    }
    case EL_HIR_TYPE_DISTINCT:
        if (type->as.distinct.orig == NULL)
            return el_mir_new_void_type(tcache->arena);
        return el_tcache_get_mir(tcache, type->as.distinct.orig);
    case EL_HIR_TYPE_QUAL:
        return el_tcache_get_mir(tcache, type->as.qual.base);
    }
    EL_UNREACHABLE("unknown hir type kind");
}

ElMirType* el_lowerer_map_type(ElLowerer* lw, const ElHirType* type) {
    el_prof_begin_sub(lw->prof, lw->pss_type);
    ElMirType* result = el_tcache_get_mir(lw->tcache, type);
    el_prof_finish_sub(lw->prof, lw->pss_type);
    return result;
}
