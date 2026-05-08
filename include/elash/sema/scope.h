#pragma once

#include <elash/defs/int-types.h>
#include <elash/defs/attr.h>
#include <elash/defs/sv.h>

#include <elash/sema/symbol.h>

typedef struct ElScope ElScope;
struct ElScope {
    ElScope* parent;

    ElSymbol** entries;
    usize capacity;
    usize count;
};

ElScope* el_sema_scope_new(ElScope* parent);
void el_sema_scope_free(ElScope* scope);

EL_ATTR_NODISCARD
bool el_sema_scope_insert(ElScope* scope, ElSymbol* symbol);

ElSymbol* el_sema_scope_lookup(ElScope* scope, ElStringView name);
ElSymbol* el_sema_scope_lookup_local(ElScope* scope, ElStringView name);
