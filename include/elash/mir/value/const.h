#pragma once

#include <elash/util/dynarena.h>
#include <elash/sema/const.h>
#include <elash/sema/type.h>

typedef struct ElMirValue ElMirValue;

typedef ElConstant ElMirConstValue;
ElMirValue* el_mir_new_const(ElDynArena* arena, ElType* type, ElConstant constant);
