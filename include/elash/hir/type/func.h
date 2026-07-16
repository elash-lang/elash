#pragma once

#include <elash/util/dynarena.h>
#include <elash/defs/int-types.h>

typedef struct ElHirType ElHirType;

typedef struct ElHirFuncType {
    ElHirType* ret_type;
    ElHirType** params;
    usize param_count;
} ElHirFuncType;

ElHirType* el_hir_new_func_type(ElDynArena* arena, ElHirType* ret_type, ElHirType** params, usize param_count);
