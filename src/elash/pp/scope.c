#include <elash/pp/scope.h>

#include <elash/defs/int-types.h>
#include <elash/util/strhash.h>

#include <stdlib.h>

typedef struct _ElPpScopeEntry Entry;

#define LOAD_FACTOR_GROW   0.7
#define LOAD_FACTOR_SHRINK 0.2

#define INITIAL_CAPACITY 16
#define MIN_CAPACITY 8

static usize next_power_of_two(usize x) {
    usize power = 1;
    while (power < x) power <<= 1;
    return power;
}

bool resize(ElPpScope* scope, usize new_capacity) {
    new_capacity = next_power_of_two(new_capacity);
    if (new_capacity < MIN_CAPACITY)
        new_capacity = MIN_CAPACITY;

    Entry* old_entries = scope->entries;
    usize old_capacity = scope->capacity;

    Entry* new_entries = calloc(new_capacity, sizeof(Entry));
    if (new_entries == NULL)
        return false;

    scope->entries = new_entries;
    scope->capacity = new_capacity;
    scope->num_entries = 0;
    scope->num_tombstones = 0;

    for (usize i = 0; i < old_capacity; ++i) {
        if (old_entries[i].state == _EL_PP_OCCUPIED) {
           el_pp_scope_assign(scope, old_entries[i].key, old_entries[i].value);
        }
    }

    free(old_entries);
    return true;
}

static bool ensure_capacity_for_new_var(ElPpScope* scope) {
    double load = (double)(scope->num_entries + scope->num_tombstones) / (double)scope->capacity;
    if (load >= LOAD_FACTOR_GROW) {
        return resize(scope, scope->capacity * 2);
    }
    return true;
}

static void maybe_shrink(ElPpScope* scope) {
    double load = (double)scope->num_entries / (double)scope->capacity;
    if (scope->capacity > MIN_CAPACITY && load <= LOAD_FACTOR_SHRINK) {
        resize(scope, scope->capacity / 2);
    }
}

static Entry* find_slot(ElPpScope* scope, ElStringView key, bool* found) {
    usize index = el_hash_string(key) & (scope->capacity - 1);
    Entry* first_tombstone = NULL;

    for (;;) {
        Entry* entry = &scope->entries[index];

        if (entry->state == _EL_PP_EMPTY) {
            *found = false;
            return first_tombstone != NULL ? first_tombstone : entry;
        }

        if (entry->state == _EL_PP_TOMBSTONE) {
            if (first_tombstone == NULL)
                first_tombstone = entry;
        } else if (el_sv_eql(entry->key, key)) {
            *found = true;
            return entry;
        }

        index = (index + 1) & (scope->capacity - 1);
    }
}

ElPpScope* el_pp_scope_new(ElPpScope* parent) {
    ElPpScope* scope = malloc(sizeof(ElPpScope));
    if (!scope) return NULL;

    scope->entries = calloc(INITIAL_CAPACITY, sizeof(Entry));
    if (!scope->entries) {
        free(scope);
        return NULL;
    }

    scope->capacity = INITIAL_CAPACITY;
    scope->num_entries = 0;
    scope->num_tombstones = 0;
    scope->parent = parent;

    return scope;
}

void el_pp_scope_free(ElPpScope* scope) {
    if (scope == NULL)
        return;

    free(scope->entries);
    free(scope);
}

bool el_pp_scope_assign(ElPpScope* scope, ElStringView key, ElPpSymbol* value) {
    if (!ensure_capacity_for_new_var(scope))
        return false;

    bool found;
    Entry* slot = find_slot(scope, key, &found);

    if (found) {
        slot->value = value;
        return true;
    }

    if (slot->state == _EL_PP_TOMBSTONE)
        scope->num_tombstones--;

    slot->key = key;
    slot->value = value;
    slot->state = _EL_PP_OCCUPIED;
    scope->num_entries++;

    return true;
}

bool el_pp_scope_deassign(ElPpScope* scope, ElStringView key) {
    bool found;
    Entry* slot = find_slot(scope, key, &found);

    if (!found)
        return false;

    slot->state = _EL_PP_TOMBSTONE;
    scope->num_entries--;
    scope->num_tombstones++;

    maybe_shrink(scope);
    return true;
}

ElPpSymbol* el_pp_scope_lookup_local(ElPpScope* scope, ElStringView key) {
    bool found;
    Entry* slot = find_slot(scope, key, &found);
    if (!found)
        return NULL;

    return slot->value;
}

ElPpSymbol* el_pp_scope_lookup(ElPpScope* scope, ElStringView key) {
    ElPpSymbol* var = el_pp_scope_lookup_local(scope, key);
    if (var != NULL)
        return var;
    if (scope->parent != NULL)
        return el_pp_scope_lookup(scope->parent, key);
    return NULL;
}

bool el_pp_scope_has(ElPpScope* scope, ElStringView key) {
    bool found;
    find_slot(scope, key, &found);
    return found;
}
