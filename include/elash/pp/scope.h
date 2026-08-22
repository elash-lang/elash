#pragma once

#include <elash/defs/sv.h>
#include <elash/pp/symbol.h>
#include <stdbool.h>

struct _ElPpScopeEntry {
    ElStringView key;
    ElPpSymbol* value;
    enum {
        _EL_PP_EMPTY,
        _EL_PP_OCCUPIED,
        _EL_PP_TOMBSTONE,
    } state;
};

typedef struct ElPpScope ElPpScope;
struct ElPpScope {
    struct _ElPpScopeEntry* entries;
    usize capacity;
    usize num_entries;
    usize num_tombstones;
    ElPpScope* parent;
};

/// @brief Allocates and initializes a new ElPpScope hash map.
ElPpScope* el_pp_scope_new(ElPpScope* parent);
/// @brief Destroys an ElPpScope hash map and frees its memory.
void el_pp_scope_free(ElPpScope* scope);

/// @brief Assigns a pointer to a symbol to a key in the hash map.
/// @return True if assignment is successful, false otherwise.
bool el_pp_scope_assign(ElPpScope* scope, ElStringView key, ElPpSymbol* value);
/// @brief Removes a key-value pair from the hash map (does not free the value).
bool el_pp_scope_deassign(ElPpScope* scope, ElStringView key);

/// @brief Retrieves the pointer to the symbol associated with a key from this scope only.
ElPpSymbol* el_pp_scope_lookup_local(ElPpScope* scope, ElStringView key);
/// @brief Retrieves the pointer to the symbol associated with a key from this scope or its parents.
ElPpSymbol* el_pp_scope_lookup(ElPpScope* scope, ElStringView key);
/// @brief Checks if a key exists in the hash map.
bool el_pp_scope_has(ElPpScope* scope, ElStringView key);
