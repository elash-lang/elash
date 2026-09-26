#include <elash/mir/instr-buf.h>
#include <elash/util/alloc.h>

#include <string.h>

static void el_mir_ibuf_reallocate(ElMirInstrBuf* ibuf, usize new_cap);

void el_mir_ibuf_init(ElMirInstrBuf* ibuf) {
    ibuf->items = NULL;
    ibuf->len = 0;
    ibuf->cap = 0;
}

void el_mir_ibuf_destroy(ElMirInstrBuf* ibuf) {
    el_free(ibuf->items);
    ibuf->items = NULL;
    ibuf->len = 0;
    ibuf->cap = 0;
}

void el_mir_ibuf_copy(const ElMirInstrBuf* src, ElMirInstrBuf* dst) {
    el_mir_ibuf_init(dst);

    if (src->len == 0) return;

    dst->items = EL_NEW_ARR(ElMirInstr*, src->len);

    memcpy(dst->items, src->items, src->len * sizeof(ElMirInstr*));
    dst->len = src->len;
    dst->cap = src->len;
}

void el_mir_ibuf_move(ElMirInstrBuf* src, ElMirInstrBuf* dst) {
    el_mir_ibuf_destroy(dst);

    dst->items = src->items;
    dst->len = src->len;
    dst->cap = src->cap;

    el_mir_ibuf_init(src);
}

static void el_mir_ibuf_reallocate(ElMirInstrBuf* ibuf, usize new_cap) {
    if (new_cap == ibuf->cap) return;

    ibuf->items = el_realloc(ibuf->items, new_cap, sizeof(ElMirInstr*));
    ibuf->cap = new_cap;
}

void el_mir_ibuf_reserve(ElMirInstrBuf* ibuf, usize min_cap) {
    if (ibuf->cap >= min_cap) {
        return;
    }

    usize new_cap = ibuf->cap == 0 ? (min_cap < 4 ? 4 : min_cap) : ibuf->cap * 2;
    if (new_cap < min_cap) {
        new_cap = min_cap;
    }

    el_mir_ibuf_reallocate(ibuf, new_cap);
}

void el_mir_ibuf_reserve_exact(ElMirInstrBuf* ibuf, usize new_cap) {
    if (ibuf->cap == new_cap) return;

    el_mir_ibuf_reallocate(ibuf, new_cap);
}

void el_mir_ibuf_resize(ElMirInstrBuf* ibuf, usize new_size) {
    if (new_size > ibuf->cap) {
        el_mir_ibuf_reserve_exact(ibuf, new_size);
    }

    if (new_size > ibuf->len) {
        memset(
            ibuf->items + ibuf->len,
            0,
            (new_size - ibuf->len) * sizeof(ElMirInstr*)
        );
    }

    ibuf->len = new_size;
}

void el_mir_ibuf_push(ElMirInstrBuf* ibuf, ElMirInstr* instr) {
    if (ibuf->len == ibuf->cap) {
        el_mir_ibuf_reserve(ibuf, ibuf->len + 1);
    }

    ibuf->items[ibuf->len++] = instr;
}

void el_mir_ibuf_clear(ElMirInstrBuf* ibuf) {
    ibuf->len = 0;
}
