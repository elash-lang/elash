#pragma once

#include <elash/defs/int-types.h>
#include <elash/util/dynarena.h>

typedef struct ElMirType ElMirType;
typedef struct ElMirFuncType {
    ElMirType* ret_type;
    ElMirType** params;
    usize param_count;
} ElMirFuncType;

ElMirType* el_mir_new_func_type(ElDynArena* arena, ElMirType* ret_type, ElMirType** params, usize param_count);
