#pragma once

#include <elash/util/dynarena.h>
#include <elash/mir/type.h>
#include <elash/mir/symbol.h>

typedef struct ElMirValue ElMirValue;
typedef struct ElMirConstant ElMirConstant;

typedef struct ElMirGlobalValue {
    ElMirSymbol* sym;
    ElMirConstant* init; // NULL for zero-init
    bool is_definition;
    bool is_constant;
} ElMirGlobalValue;

ElMirValue* el_mir_new_global(
    ElDynArena* arena, ElMirType* type,
    ElMirSymbol* global, ElMirConstant* init, bool is_definition, bool is_constant
);
