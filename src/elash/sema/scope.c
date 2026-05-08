#include <elash/sema/scope.h>
#include <elash/util/strhash.h>

#include <stdlib.h>

#define INITIAL_CAPACITY 16
#define LOAD_FACTOR 0.75

ElScope* el_sema_scope_new(ElScope* parent) {
    ElScope* scope = malloc(sizeof(ElScope));
    if (!scope) return NULL;

    scope->parent = parent;
    scope->capacity = INITIAL_CAPACITY;
    scope->count = 0;
    scope->entries = calloc(scope->capacity, sizeof(ElSymbol*));
    
    if (!scope->entries) {
        free(scope);
        return NULL;
    }

    return scope;
}

void el_sema_scope_free(ElScope* scope) {
    if (!scope) return;
    free(scope->entries);
    free(scope);
}

static bool resize(ElScope* scope) {
    usize old_capacity = scope->capacity;
    ElSymbol** old_entries = scope->entries;

    scope->capacity *= 2;
    scope->entries = calloc(scope->capacity, sizeof(ElSymbol*));
    if (!scope->entries) {
        scope->entries = old_entries;
        scope->capacity = old_capacity;
        return false;
    }

    scope->count = 0;
    for (usize i = 0; i < old_capacity; i++) {
        if (old_entries[i]) {
            (void) el_sema_scope_insert(scope, old_entries[i]);
        }
    }

    free(old_entries);
    return true;
}

bool el_sema_scope_insert(ElScope* scope, ElSymbol* symbol) {
    if ((double)scope->count / (double)scope->capacity >= LOAD_FACTOR) {
        if (!resize(scope)) return false;
    }

    ulong hash = el_hash_string(symbol->name);
    usize index = hash % scope->capacity;

    while (scope->entries[index]) {
        if (el_sv_eql(scope->entries[index]->name, symbol->name)) {
            return false; // already exists
        }
        index = (index + 1) % scope->capacity;
    }

    scope->entries[index] = symbol;
    scope->count++;
    return true;
}

ElSymbol* el_sema_scope_lookup_local(ElScope* scope, ElStringView name) {
    if (scope->count == 0) return NULL;

    ulong hash = el_hash_string(name);
    usize index = hash % scope->capacity;

    while (scope->entries[index]) {
        if (el_sv_eql(scope->entries[index]->name, name)) {
            return scope->entries[index];
        }
        index = (index + 1) % scope->capacity;
    }

    return NULL;
}

ElSymbol* el_sema_scope_lookup(ElScope* scope, ElStringView name) {
    while (scope) {
        ElSymbol* symbol = el_sema_scope_lookup_local(scope, name);
        if (symbol) return symbol;
        scope = scope->parent;
    }
    return NULL;
}
