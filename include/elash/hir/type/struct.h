#pragma once

#include <elash/util/dynarena.h>
#include <elash/srcdoc/span.h>

typedef struct ElHirType ElHirType;

typedef struct ElHirStructField {
    ElStringView name;
    ElHirType* type;
} ElHirStructField;

typedef struct ElHirStructType {
    ElHirStructField* fields;
    usize count;
} ElHirStructType;

ElHirType* el_hir_new_struct_type(ElDynArena* arena, ElHirStructField* fields, usize count);
