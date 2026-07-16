#pragma once

#include <elash/util/dynarena.h>
#include <elash/mir/type.h>

typedef struct ElMirValue ElMirValue;

typedef struct ElMirConstant {
    union {
        int64_t int_;
    } as;
} ElMirConstant;

typedef ElMirConstant ElMirConstValue;
ElMirValue* el_mir_new_const(ElDynArena* arena, ElMirType* type, ElMirConstant constant);
