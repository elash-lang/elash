#pragma once

#include <elash/util/dynarena.h>
#include <elash/defs/int-types.h>

typedef struct ElHirExprNode ElHirExprNode;
typedef struct ElType ElType;

typedef struct ElHirArrayLitNode {
    ElHirExprNode** values;
    usize count;
} ElHirArrayLitNode;

ElHirExprNode* el_hir_new_array_lit(ElDynArena* arena, ElType* type, ElHirExprNode** values, usize count);
