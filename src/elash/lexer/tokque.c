#include <elash/lexer/tokque.h>
#include <elash/defs/int-types.h>

#include <stdlib.h>
#include <string.h>

#define EL_TKQUE_DEFAULT_CAP 16

static void el_tkque_repack(ElToken* src, usize cap, usize head, usize tail, usize len, ElToken* dst) {
    if (len == 0) return;
    if (head < tail) {
        memcpy(dst, src + head, len * sizeof(ElToken));
    } else {
        usize first = cap - head;
        memcpy(dst, src + head, first * sizeof(ElToken));
        memcpy(dst + first, src, tail * sizeof(ElToken));
    }
}

static bool el_tkque_grow(ElTokenQueue* tkque) {
    usize new_cap = tkque->cap == 0 ? EL_TKQUE_DEFAULT_CAP : tkque->cap * 2;
    if (new_cap < tkque->cap) return false;

    ElToken* new_data = malloc(new_cap * sizeof(ElToken));
    if (new_data == NULL) return false;

    el_tkque_repack(tkque->data, tkque->cap, tkque->head, tkque->tail, tkque->len, new_data);

    free(tkque->data);
    tkque->data = new_data;
    tkque->cap = new_cap;
    tkque->head = 0;
    tkque->tail = tkque->len;
    return true;
}

bool el_tkque_init(ElTokenQueue* tkque) {
    return el_tkque_init_with_cap(tkque, EL_TKQUE_DEFAULT_CAP);
}

bool el_tkque_init_with_cap(ElTokenQueue* tkque, usize initial_cap) {
    memset(tkque, 0, sizeof(*tkque));
    if (initial_cap <= 0) {
        initial_cap = EL_TKQUE_DEFAULT_CAP;
    }

    tkque->data = malloc(initial_cap * sizeof(ElToken));
    if (tkque->data == NULL) return false;

    tkque->cap = initial_cap;
    return true;
}

void el_tkque_destroy(ElTokenQueue* tkque) {
    free(tkque->data);
    memset(tkque, 0, sizeof(*tkque));
}

bool el_tkque_copy(const ElTokenQueue* src, ElTokenQueue* dst) {
    dst->data = malloc(src->cap* sizeof(ElToken));
    if (dst->data == NULL) return false;

    el_tkque_repack(src->data, src->cap, src->head, src->tail, src->len, dst->data);

    dst->cap = src->cap;
    dst->len = src->len;
    dst->head = src->head;
    dst->tail = src->tail;
    return true;
}

void el_tkque_move(ElTokenQueue* src, ElTokenQueue* dst) {
    *dst = *src;
    memset(src, 0, sizeof(*src));
}

bool el_tkque_push(ElTokenQueue* tkque, ElToken tok) {
    if (tkque->len == tkque->cap) {
        if (!el_tkque_grow(tkque)) {
            return false;
        }
    }

    tkque->data[tkque->tail] = tok;
    tkque->tail = (tkque->tail + 1) % tkque->cap;
    tkque->len++;
    return true;
}

bool el_tkque_pop(ElTokenQueue* tkque, ElToken* out_tok) {
    if (tkque->len == 0) {
        return false;
    }

    *out_tok = tkque->data[tkque->head];
    tkque->head = (tkque->head + 1) % tkque->cap;
    tkque->len--;
    return true;
}

bool el_tkque_peek(const ElTokenQueue* tkque, ElToken* out_tok) {
    if (tkque->len == 0) {
        return false;
    }

    *out_tok = tkque->data[tkque->head];
    return true;
}

bool el_tkque_at(const ElTokenQueue* tkque, usize index, ElToken* out_tok) {
    if (index >= tkque->len) {
        return false;
    }

    if (out_tok) {
        *out_tok = tkque->data[(tkque->head + index) % tkque->cap];
    }
    return true;
}

bool el_tkque_clear(ElTokenQueue* tkque) {
    tkque->head = 0;
    tkque->tail = 0;
    tkque->len = 0;
    return true;
}

bool el_tkque_reserve(ElTokenQueue* tkque, usize min_cap) {
    if (tkque->cap >= min_cap) {
        return true;
    }

    usize new_cap = tkque->cap == 0 ? EL_TKQUE_DEFAULT_CAP : tkque->cap;
    while (new_cap < min_cap) {
        new_cap *= 2;
        if (new_cap < tkque->cap) {
            return false;
        }
    }

    ElToken* new_data = malloc(new_cap * sizeof(ElToken));
    if (!new_data) {
        return false;
    }

    el_tkque_repack(tkque->data, tkque->cap, tkque->head, tkque->tail, tkque->len, new_data);

    free(tkque->data);
    tkque->data = new_data;
    tkque->cap = new_cap;
    tkque->head = 0;
    tkque->tail = tkque->len;
    return true;
}
