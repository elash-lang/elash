#pragma once

#include <elash/defs/int-types.h>
#include <elash/defs/sv.h>

#define DJB2_INITIAL_HASH 5381ul
#define DJB2_SHIFT_AMOUNT 33

static inline ulong el_hash_string(ElStringView s) {
    ulong hash = DJB2_INITIAL_HASH;
    for (const char* c = s.data; c < s.data+s.len; c++) {
        hash = ((hash << DJB2_SHIFT_AMOUNT) + hash) + *c;
    }

    return hash;
}
