#pragma once

#include <elash/defs/sv.h>
#include <elash/mir/type.h>
#include <elash/util/dynarena.h>

typedef enum ElMirSymbolKind {
    EL_MIR_SYM_VAR,
    EL_MIR_SYM_FUNC,
} ElMirSymbolKind;

typedef struct ElMirSymbol ElMirSymbol;

typedef struct ElMirVarSymbol {
    ElMirType* type;
} ElMirVarSymbol;

typedef struct ElMirFuncSymbol {
    ElMirType* type;
    ElMirSymbol** params;
    usize param_count;
    bool is_defined;
} ElMirFuncSymbol;

struct ElMirSymbol {
    ElStringView name;
    ElMirSymbolKind kind;
    uint32_t id;
    union {
        ElMirVarSymbol var;
        ElMirFuncSymbol func;
    } as;
};

ElMirSymbol* el_mir_new_var_symbol(ElDynArena* arena, uint32_t id, ElStringView name, ElMirType* type);
ElMirSymbol* el_mir_new_func_symbol(
    ElDynArena* arena, uint32_t id, ElStringView name, ElMirType* ret_type, ElMirSymbol** params, usize param_count
);
void el_mir_dump_symbol(const ElMirSymbol* symbol, FILE* out);
