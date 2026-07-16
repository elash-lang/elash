#pragma once

#include <elash/util/dynarena.h>
#include <elash/mir/type.h>
#include <elash/mir/symbol.h>

typedef struct ElMirValue ElMirValue;

typedef struct ElMirGlobalValue {
    ElMirSymbol* sym;
} ElMirGlobalValue;

ElMirValue* el_mir_new_global(ElDynArena* arena, ElMirType* type, ElMirSymbol* global);
