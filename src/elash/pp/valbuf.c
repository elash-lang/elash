#include <elash/pp/valbuf.h>
#include <elash/pp/value.h>
#include <elash/util/alloc.h>

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

void el_pp_valbuf_reserve(ElPpValBuf* vbuf, usize new_cap) {
    if (new_cap <= vbuf->cap)
        return;

    vbuf->data = el_realloc(vbuf->data, new_cap, sizeof(ElPpValue*));
    vbuf->cap = new_cap;
}

void el_pp_valbuf_init(ElPpValBuf* vbuf) {
    vbuf->data = NULL;
    vbuf->count = 0;
    vbuf->cap = 0;
}

void el_pp_valbuf_free(ElPpValBuf* vbuf) {
    el_free(vbuf->data);
    vbuf->data = NULL;
    vbuf->count = 0;
    vbuf->cap = 0;
}

void el_pp_valbuf_clear(ElPpValBuf* vbuf) {
    vbuf->count = 0;
}

void el_pp_valbuf_push(ElPpValBuf* vbuf, ElPpValue* val) {
    if (vbuf->count == vbuf->cap) {
        usize new_cap = vbuf->cap ? vbuf->cap * 2 : 4;
        el_pp_valbuf_reserve(vbuf, new_cap);
    }

    vbuf->data[vbuf->count] = val;
    vbuf->count++;
}

ElPpList el_pp_valbuf_flush(ElPpValBuf* vbuf, ElDynArena* arena) {
    ElPpValue** values = NULL;
    if (vbuf->count != 0) {
        values = EL_DYNARENA_NEW_ARR(
            arena, ElPpValue*, vbuf->count
        );
        memcpy(values, vbuf->data, vbuf->count * sizeof(ElPpValue*));
    }

    usize count = vbuf->count;
    el_pp_valbuf_free(vbuf);

    return (ElPpList) {
        .values = values,
        .count  = count,
    };
}

ElPpValue* el_pp_valbuf_vflush(ElPpValBuf* vbuf, ElDynArena* arena) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElPpValue, {
        .type = EL_PP_TYPE_LIST,
        .as.list_ = el_pp_valbuf_flush(vbuf, arena),
    });
}
