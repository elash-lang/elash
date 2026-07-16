#pragma once

#include <elash/mir/type.h>

#include "value/global.h"
#include "value/const.h"
#include "value/reg.h"
#include "value/arg.h"

typedef enum ElMirValueKind {
    EL_MIR_VAL_REG,
    EL_MIR_VAL_CONST,
    EL_MIR_VAL_ARG,
    EL_MIR_VAL_GLOBAL,
} ElMirValueKind;

typedef struct ElMirValue {
    ElMirValueKind kind;
    ElMirType* type;
    union {
        ElMirRegValue    reg;
        ElMirArgValue    arg;
        ElMirConstValue  constant;
        ElMirGlobalValue global;
    } as;
} ElMirValue;
