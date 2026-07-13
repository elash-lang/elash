#pragma once

#include <stdint.h>
#include <elash/util/dynarena.h>

typedef struct ElHirExpr ElHirExpr;
typedef struct ElType ElType;

ElHirExpr* el_hir_new_int_constant(ElDynArena* arena, ElType* type, int64_t value);
ElHirExpr* el_hir_new_char_constant(ElDynArena* arena, ElType* type, char value);
ElHirExpr* el_hir_new_bool_constant(ElDynArena* arena, ElType* type, bool value);
