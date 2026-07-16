#pragma once

#include <elash/util/dynarena.h>
#include <elash/mir/type.h>

typedef struct ElLowererBuiltins {
    // TODO: add more types (if needed)
    ElMirType* type_usize;
    ElMirType* type_void;
} ElLowererBuiltins;

void el_lowerer_init_builtins(ElLowererBuiltins* builtins, ElDynArena* arena);
