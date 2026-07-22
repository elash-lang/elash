#include <elash/lowerer/lowerer.h>

#include <elash/util/assert.h>
#include <elash/util/todo.h>
#include <elash/mir/type.h>

#define CHAR_BITS 8 // one byte

static ElMirType* make_slice_type(ElLowerer* lw, const ElHirSliceType* slice) {
    ElMirType** items = EL_DYNARENA_NEW_ARR(lw->arena, ElMirType*, 2);
    items[EL_MIR_SLICE_FIELD_DATA] = el_mir_new_ptr_type(lw->arena, el_lowerer_map_type(lw, slice->base));
    items[EL_MIR_SLICE_FIELD_LEN]  = lw->builtins->type_usize;
    return el_mir_new_tuple_type(lw->arena, items, 2);
}

ElMirType* el_lowerer_map_type(ElLowerer* lw, const ElHirType* type) {
    if (type == NULL) return NULL;
    switch (type->kind) {
    case EL_HIR_TYPE_PRIM:
        switch (type->as.prim.kind) {
        case EL_PRIMTYPE_VOID:
            return el_mir_new_void_type(lw->arena);
        case EL_PRIMTYPE_BOOL:
            return el_mir_new_int_type(lw->arena, 1, false);
        case EL_PRIMTYPE_CHAR:
            return el_mir_new_int_type(lw->arena, CHAR_BITS, false);
        case EL_PRIMTYPE_INT: {
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
            return el_mir_new_int_type(lw->arena, width, type->as.prim.as.integral.is_signed);
        }
        }
        EL_UNREACHABLE("unknown prim type kind");
    case EL_HIR_TYPE_FUNC: {
        ElMirType* ret_type = el_lowerer_map_type(lw, type->as.func.ret_type);
        ElMirType** params = EL_DYNARENA_NEW_ARR(lw->arena, ElMirType*, type->as.func.param_count);
        for (usize i = 0; i < type->as.func.param_count; ++i) {
            params[i] = el_lowerer_map_type(lw, type->as.func.params[i]);
        }
        return el_mir_new_func_type(lw->arena, ret_type, params, type->as.func.param_count);
    }
    case EL_HIR_TYPE_ARRAY: {
        ElMirType* base = el_lowerer_map_type(lw, type->as.array.base);
        return el_mir_new_array_type(lw->arena, base, type->as.array.size);
    }
    case EL_HIR_TYPE_STRUCT: {
        ElMirType** elements = EL_DYNARENA_NEW_ARR(lw->arena, ElMirType*, type->as.struct_.count);
        for (usize i = 0; i < type->as.struct_.count; ++i) {
            elements[i] = el_lowerer_map_type(lw, type->as.struct_.fields[i].type);
        }
        return el_mir_new_tuple_type(lw->arena, elements, type->as.struct_.count);
    }
    case EL_HIR_TYPE_TUPLE: {
        ElMirType** elements = EL_DYNARENA_NEW_ARR(lw->arena, ElMirType*, type->as.tuple.count);
        for (usize i = 0; i < type->as.tuple.count; ++i) {
            elements[i] = el_lowerer_map_type(lw, type->as.tuple.elements[i]);
        }
        return el_mir_new_tuple_type(lw->arena, elements, type->as.tuple.count);
    }
    case EL_HIR_TYPE_REF:
        return el_mir_new_ptr_type(lw->arena, el_lowerer_map_type(lw, type->as.ref.base));
    case EL_HIR_TYPE_RWSLICE:
        return el_mir_new_ptr_type(lw->arena, el_lowerer_map_type(lw, type->as.rwslice.base));
    case EL_HIR_TYPE_SLICE:
        return make_slice_type(lw, &type->as.slice);
    }
    EL_UNREACHABLE("unknown hir type kind");
}
