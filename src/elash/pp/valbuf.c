#include <elash/pp/valbuf.h>
#include <elash/pp/value.h>

#include <stdbool.h>
#include <stdlib.h>

bool el_pp_valbuf_reserve(ElPpValBuf* vbuf, usize new_cap) {
    if (new_cap <= vbuf->cap)
        return true;

    ElPpValue** new_data = realloc(vbuf->data, new_cap * sizeof(ElPpValue*));
    if (new_data == NULL) return false;

    vbuf->data = new_data;
    vbuf->cap = new_cap;
    return true;
}

bool el_pp_valbuf_init(ElPpValBuf* vbuf) {
    vbuf->data = NULL;
    vbuf->count = 0;
    vbuf->cap = 0;
    return true;
}

void el_pp_valbuf_free(ElPpValBuf* vbuf) {
    free(vbuf->data);
    vbuf->data = NULL;
    vbuf->count = 0;
    vbuf->cap = 0;
}

void el_pp_valbuf_clear(ElPpValBuf* vbuf) {
    vbuf->count = 0;
}

bool el_pp_valbuf_push(ElPpValBuf* vbuf, ElPpValue* val) {
    if (vbuf->count == vbuf->cap) {
        usize new_cap = vbuf->cap ? vbuf->cap * 2 : 4;
        if (!el_pp_valbuf_reserve(vbuf, new_cap))
            return false;
    }

    vbuf->data[vbuf->count] = val;
    vbuf->count++;
    return true;
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
