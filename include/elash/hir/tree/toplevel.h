#pragma once

#include "decl.h"

typedef enum ElHirTopLevelKind {
    EL_HIR_TOPLVL_DECL,
} ElHirTopLevelKind;

typedef struct ElHirTopLevel ElHirTopLevel;
struct ElHirTopLevel {
    ElHirTopLevelKind kind;
    union {
        ElHirDecl* decl;
    } as;
    ElHirTopLevel* next;
};

ElHirTopLevel* el_hir_new_toplevel_decl(ElDynArena* arena, ElHirDecl* decl);
