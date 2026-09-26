#pragma once

#include <elash/defs/int-types.h>
#include <stdbool.h>

typedef struct ElGHMEntry ElGHMEntry;

typedef bool ElGHMEqualFn(const void*, const void*);
typedef uhash ElGHMHashFn(const void*);

// GHM - Generic Hash Map
// Only for storing pointers (applies to both key and value)
typedef struct ElGHM {
    ElGHMEntry* entries;
    usize capacity;
    usize count;
    usize tombstones;

    ElGHMHashFn* hash;
    ElGHMEqualFn* eql;
} ElGHM;

void el_ghm_init(ElGHM* ghm, ElGHMHashFn* hash, ElGHMEqualFn* eql);
void el_ghm_free(ElGHM* ghm);

void el_ghm_insert(ElGHM* ghm, const void* key, void* value);
void* el_ghm_lookup(ElGHM* ghm, const void* key);
bool el_ghm_remove(ElGHM* ghm, const void* key);
