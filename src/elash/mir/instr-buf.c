#include <elash/mir/instr-buf.h>

#include <stdlib.h>
#include <string.h>

static bool el_mir_ibuf_reallocate(ElMirInstrBuf* ibuf, usize new_cap);

bool el_mir_ibuf_init(ElMirInstrBuf* ibuf) {
    ibuf->items = NULL;
    ibuf->len = 0;
    ibuf->cap = 0;
    return true;
}

void el_mir_ibuf_destroy(ElMirInstrBuf* ibuf) {
    free(ibuf->items);
    ibuf->items = NULL;
    ibuf->len = 0;
    ibuf->cap = 0;
}

bool el_mir_ibuf_copy(const ElMirInstrBuf* src, ElMirInstrBuf* dst) {
    el_mir_ibuf_init(dst);

    if (src->len == 0) return true;

    dst->items = malloc(src->len * sizeof(ElMirInstr*));
    if (dst->items == NULL) {
        return false;
    }

    memcpy(dst->items, src->items, src->len * sizeof(ElMirInstr*));
    dst->len = src->len;
    dst->cap = src->len;
    return true;
}

bool el_mir_ibuf_move(ElMirInstrBuf* src, ElMirInstrBuf* dst) {
    el_mir_ibuf_destroy(dst);

    dst->items = src->items;
    dst->len = src->len;
    dst->cap = src->cap;

    el_mir_ibuf_init(src);
    return true;
}

static bool el_mir_ibuf_reallocate(ElMirInstrBuf* ibuf, usize new_cap) {
    if (new_cap == ibuf->cap) return true;

    ElMirInstr** new_items = realloc(ibuf->items, new_cap * sizeof(ElMirInstr*));
    if (new_items == NULL) return false;

    ibuf->items = new_items;
    ibuf->cap = new_cap;
    return true;
}

bool el_mir_ibuf_reserve(ElMirInstrBuf* ibuf, usize min_cap) {
    if (ibuf->cap >= min_cap) {
        return true;
    }

    usize new_cap = ibuf->cap == 0 ? (min_cap < 4 ? 4 : min_cap) : ibuf->cap * 2;
    if (new_cap < min_cap) {
        new_cap = min_cap;
    }

    return el_mir_ibuf_reallocate(ibuf, new_cap);
}

bool el_mir_ibuf_reserve_exact(ElMirInstrBuf* ibuf, usize new_cap) {
    if (new_cap < ibuf->len)  return false;
    if (ibuf->cap == new_cap) return true;

    return el_mir_ibuf_reallocate(ibuf, new_cap);
}

bool el_mir_ibuf_resize(ElMirInstrBuf* ibuf, usize new_size) {
    if (new_size > ibuf->cap) {
        if (!el_mir_ibuf_reserve_exact(ibuf, new_size)) {
            return false;
        }
    }

    if (new_size > ibuf->len) {
        memset(
            ibuf->items + ibuf->len,
            0,
            (new_size - ibuf->len) * sizeof(ElMirInstr*)
        );
    }

    ibuf->len = new_size;
    return true;
}

bool el_mir_ibuf_push(ElMirInstrBuf* ibuf, ElMirInstr* instr) {
    if (ibuf->len == ibuf->cap) {
        if (!el_mir_ibuf_reserve(ibuf, ibuf->len + 1)) {
            return false;
        }
    }

    ibuf->items[ibuf->len++] = instr;
    return true;
}

bool el_mir_ibuf_clear(ElMirInstrBuf* ibuf) {
    ibuf->len = 0;
    return true;
}
