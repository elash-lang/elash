#pragma once

#include <elash/defs/int-types.h>
#include <elash/defs/attr.h>
#include <elash/defs/sv.h>

#include <stdbool.h>
#include <stdalign.h>

typedef struct _ElDynArenaChunk _ElDynArenaChunk;
struct _ElDynArenaChunk {
    _ElDynArenaChunk* next;
    usize size;
    uchar data[];
};

typedef struct ElDynArena {
    _ElDynArenaChunk* head;
    _ElDynArenaChunk* current;
    usize offset;
} ElDynArena;

bool el_dynarena_init(ElDynArena* arena);
void el_dynarena_free(ElDynArena* arena);
void el_dynarena_reset(ElDynArena* arena);

EL_ATTR_MALLOC void* el_dynarena_alloc(ElDynArena* arena, usize size, usize align);
EL_ATTR_MALLOC void* el_dynarena_alloc_zeroed(ElDynArena* arena, usize size, usize align);

ElStringView el_dynarena_clone_sv(ElDynArena* arena, ElStringView sv);
char* el_dynarena_make_cstr(ElDynArena* arena, ElStringView sv);

#define EL_DYNARENA_NEW(ARENA, TYPE) \
    (TYPE*)(el_dynarena_alloc((ARENA), sizeof(TYPE), alignof(TYPE)))

#define EL_DYNARENA_NEW_ARR(ARENA, TYPE, SIZE) \
    (TYPE*)(el_dynarena_alloc((ARENA), sizeof(TYPE) * (SIZE), alignof(TYPE)))

#define EL_DYNARENA_NEW_ZEROED(ARENA, TYPE) \
    (TYPE*)(el_dynarena_alloc_zeroed((ARENA), sizeof(TYPE), alignof(TYPE)))

#define EL_DYNARENA_NEW_ARR_ZEROED(ARENA, TYPE, SIZE) \
     (TYPE*)(el_dynarena_alloc_zeroed((ARENA), sizeof(TYPE) * (SIZE), alignof(TYPE)))
