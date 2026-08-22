#pragma once

#include <elash/defs/int-types.h>
#include <elash/util/dynarena.h>

#include <elash/pp/value.h>
#include <stdbool.h>

typedef struct ElPpValBuf {
    ElPpValue** data;
    usize count;
    usize cap;
} ElPpValBuf;

bool el_pp_valbuf_init(ElPpValBuf* vbuf);
void el_pp_valbuf_free(ElPpValBuf* vbuf);

/// @brief Adds a value to the end of the array.
bool el_pp_valbuf_push(ElPpValBuf* vbuf, ElPpValue* val);
/// @brief Clears all elements from the array, freeing their resources but not the array's capacity.
void el_pp_valbuf_clear(ElPpValBuf* vbuf);

ElPpList el_pp_valbuf_flush(ElPpValBuf* vbuf, ElDynArena* arena);
ElPpValue* el_pp_valbuf_vflush(ElPpValBuf* vbuf, ElDynArena* arena);
