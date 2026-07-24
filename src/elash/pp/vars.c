#include <elash/pp/valarr.h>
#include <elash/pp/vars.h>

#include <elash/defs/int-types.h>
#include <elash/util/strhash.h>

#include <stdlib.h>

typedef struct _ElPpVarsEntry Entry;

#define LOAD_FACTOR_GROW   0.7
#define LOAD_FACTOR_SHRINK 0.2
#define MIN_CAPACITY 8

static usize next_power_of_two(usize x) {
    usize power = 1;
    while (power < x) power <<= 1;
    return power;
}

bool _el_pp_vars_resize(ElPpVars* vars, usize new_capacity) {
    new_capacity = next_power_of_two(new_capacity);
    if (new_capacity < MIN_CAPACITY)
        new_capacity = MIN_CAPACITY;

    Entry* old_entries = vars->entries;
    usize old_capacity = vars->capacity;

    Entry* new_entries = calloc(new_capacity, sizeof(Entry));
    if (!new_entries)
        return false;

    vars->entries = new_entries;
    vars->capacity = new_capacity;
    vars->num_entries = 0;
    vars->num_tombstones = 0;

    for (usize i = 0; i < old_capacity; ++i) {
        if (old_entries[i].state == _EL_PP_VARS_OCCUPIED) {
            el_pp_vars_assign_move(vars, old_entries[i].key, &old_entries[i].value);
        }
    }

    free(old_entries);
    return true;
}

bool _el_pp_vars_ensure_capacity_for_new_var(ElPpVars* vars) {
    double load = (double)(vars->num_entries + vars->num_tombstones) / (double)vars->capacity;
    if (load >= LOAD_FACTOR_GROW) {
        return _el_pp_vars_resize(vars, vars->capacity * 2);
    }
    return true;
}

void _el_pp_vars_maybe_shrink(ElPpVars* vars) {
    double load = (double)vars->num_entries / (double)vars->capacity;
    if (vars->capacity > MIN_CAPACITY && load <= LOAD_FACTOR_SHRINK) {
        _el_pp_vars_resize(vars, vars->capacity / 2);
    }
}

Entry* _el_pp_vars_find_slot(ElPpVars* vars, ElStringView key, bool* found) {
    usize index = el_hash_string(key) & (vars->capacity - 1);
    Entry* first_tombstone = NULL;

    for (;;) {
        Entry* entry = &vars->entries[index];

        if (entry->state == _EL_PP_VARS_EMPTY) {
            *found = false;
            return first_tombstone ? first_tombstone : entry;
        }

        if (entry->state == _EL_PP_VARS_TOMBSTONE) {
            if (!first_tombstone)
                first_tombstone = entry;
        } else if (el_sv_eql(entry->key, key)) {
            *found = true;
            return entry;
        }

        index = (index + 1) & (vars->capacity - 1);
    }
}


bool el_pp_vars_init(ElPpVars* vars, usize initial_capacity) {
    if (!vars) return false;

    initial_capacity = next_power_of_two(initial_capacity);
    if (initial_capacity < MIN_CAPACITY)
        initial_capacity = MIN_CAPACITY;

    vars->entries = calloc(initial_capacity, sizeof(Entry));
    if (!vars->entries)
        return false;

    vars->capacity = initial_capacity;
    vars->num_entries = 0;
    vars->num_tombstones = 0;

    return true;
}

void el_pp_vars_destroy(ElPpVars* vars) {
    if (!vars || !vars->entries)
        return;

    for (usize i = 0; i < vars->capacity; ++i) {
        if (vars->entries[i].state == _EL_PP_VARS_OCCUPIED) {
            el_pp_var_free(&vars->entries[i].value);
        }
    }

    free(vars->entries);
    vars->entries = NULL;
    vars->capacity = 0;
    vars->num_entries = 0;
    vars->num_tombstones = 0;
}

bool el_pp_vars_assign(ElPpVars* vars, ElStringView key, const ElPpVar* value) {
    if (!_el_pp_vars_ensure_capacity_for_new_var(vars))
        return false;

    bool found;
    Entry* slot = _el_pp_vars_find_slot(vars, key, &found);

    if (found) {
        el_pp_var_free(&slot->value);
        return el_pp_var_copy(value, &slot->value);
    }

    if (slot->state == _EL_PP_VARS_TOMBSTONE)
        vars->num_tombstones--;

    slot->key = key;
    if (!el_pp_var_copy(value, &slot->value))
        return false;
    slot->state = _EL_PP_VARS_OCCUPIED;
    vars->num_entries++;

    return true;
}

bool el_pp_vars_assign_move(ElPpVars* vars, ElStringView key, ElPpVar* value) {
    if (!_el_pp_vars_ensure_capacity_for_new_var(vars))
        return false;

    bool found;
    Entry* slot = _el_pp_vars_find_slot(vars, key, &found);

    if (found) {
        el_pp_var_free(&slot->value);
        el_pp_var_move(value, &slot->value);
        return true;
    }

    if (slot->state == _EL_PP_VARS_TOMBSTONE)
        vars->num_tombstones--;

    slot->key = key;
    el_pp_var_move(value, &slot->value);
    slot->state = _EL_PP_VARS_OCCUPIED;
    vars->num_entries++;

    return true;
}

bool el_pp_vars_deassign(ElPpVars* vars, ElStringView key) {
    bool found;
    Entry* slot = _el_pp_vars_find_slot(vars, key, &found);

    if (!found)
        return false;

    el_pp_var_free(&slot->value);
    slot->state = _EL_PP_VARS_TOMBSTONE;
    vars->num_entries--;
    vars->num_tombstones++;

    _el_pp_vars_maybe_shrink(vars);
    return true;
}

bool el_pp_vars_get(ElPpVars* vars, ElStringView key, ElPpVar* out) {
    bool found;
    Entry* slot = _el_pp_vars_find_slot(vars, key, &found);
    if (!found)
        return false;

    el_pp_var_copy(&slot->value, out);
    return true;
}

bool el_pp_vars_has(ElPpVars* vars, ElStringView key) {
    bool found;
    _el_pp_vars_find_slot(vars, key, &found);
    return found;
}
