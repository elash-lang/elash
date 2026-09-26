#include <elash/util/ghm.h>
#include <elash/util/assert.h>
#include <elash/util/alloc.h>

#include <string.h>

#define INITIAL_CAPACITY 16

typedef enum {
    EMPTY = 0,
    OCCUPIED,
    TOMBSTONE,
} EntryState;

typedef struct ElGHMEntry {
    const void* key;
    void* value;
    EntryState state;
} Entry;

// returns -1 if not found (if !for_insertion)
static isize find_index(ElGHM* ghm, const void* key, bool for_insertion) {
    uhash hash = ghm->hash(key);
    usize mask = ghm->capacity - 1;
    usize index = hash & mask;
    isize first_tombstone = -1;

    for (usize i = 0; i < ghm->capacity; i++) {
        usize cur = (index + i) & mask;
        EntryState state = ghm->entries[cur].state;

        switch (state) {
        case EMPTY:
            return (for_insertion && first_tombstone != -1) ? first_tombstone : (isize)cur;

        case TOMBSTONE:
            if (for_insertion && first_tombstone == -1) {
                first_tombstone = (isize)cur;
            }
            continue;

        case OCCUPIED:
            if (ghm->eql(ghm->entries[cur].key, key)) {
                return (isize)cur;
            }
        }
    }

    return for_insertion ? first_tombstone : -1;
}

void el_ghm_init(ElGHM* ghm, ElGHMHashFn* hash, ElGHMEqualFn* eql) {
    ghm->capacity = INITIAL_CAPACITY;
    ghm->count = 0;
    ghm->tombstones = 0;

    ghm->entries = EL_NEW_ARR_ZEROED(Entry, ghm->capacity);

    ghm->hash = hash;
    ghm->eql = eql;
}

void el_ghm_free(ElGHM* ghm) {
    if (ghm == NULL) return;
    el_free(ghm->entries);
}

static bool resize(ElGHM* ghm) {
    usize old_capacity = ghm->capacity;
    Entry* old_entries = ghm->entries;

    ghm->capacity *= 2;
    ghm->entries = EL_NEW_ARR_ZEROED(Entry, ghm->capacity);
    if (ghm->entries == NULL) {
        ghm->entries = old_entries;
        ghm->capacity = old_capacity;
        return false;
    }

    ghm->count = 0;
    ghm->tombstones = 0;

    for (usize i = 0; i < old_capacity; i++) {
        if (old_entries[i].state == OCCUPIED) {
            el_ghm_insert(ghm, old_entries[i].key, old_entries[i].value);
        }
    }

    el_free(old_entries);
    return true;
}

bool el_ghm_insert(ElGHM* ghm, const void* key, void* value) {
    if ((ghm->count + ghm->tombstones) * 4 >= ghm->capacity * 3) {
        if (!resize(ghm)) return false;
    }

    isize index = find_index(ghm, key, true);
    EL_ASSERT(index != -1, "should not happen");

    if (ghm->entries[index].state == OCCUPIED) {
        ghm->entries[index].value = value;
        return true;
    }

    if (ghm->entries[index].state == TOMBSTONE) {
        ghm->tombstones--;
    }

    ghm->entries[index].key = key;
    ghm->entries[index].value = value;
    ghm->entries[index].state = OCCUPIED;
    ghm->count++;
    return true;
}

void* el_ghm_lookup(ElGHM* ghm, const void* key) {
    isize index = find_index(ghm, key, false);
    if (index != -1 && ghm->entries[index].state == OCCUPIED) {
        return ghm->entries[index].value;
    }

    return NULL;
}

bool el_ghm_remove(ElGHM* ghm, const void* key) {
    isize index = find_index(ghm, key, false);
    if (index != -1 && ghm->entries[index].state == OCCUPIED) {
        ghm->entries[index].state = TOMBSTONE;
        ghm->count--;
        ghm->tombstones++;
        return true;
    }
    return false;
}
