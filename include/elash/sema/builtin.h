#pragma once

#include <elash/util/dynarena.h>
#include <elash/sema/type.h>

typedef struct ElBuiltins {
    ElType* type_int;
    ElType* type_uint;
    ElType* type_char;
    ElType* type_bool;
    ElType* type_void;
} ElBuiltins;

void el_sema_init_builtins(ElBuiltins* builtins, ElDynArena* arena);
