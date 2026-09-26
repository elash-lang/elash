#include <elash/lexer/tokque.h>
#include <elash/defs/int-types.h>
#include <elash/util/alloc.h>

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

static void el_tkque_grow(ElTokenQueue* tkque) {
    usize new_cap = tkque->cap == 0 ? EL_TKQUE_DEFAULT_CAP : tkque->cap * 2;

    ElToken* new_data = EL_NEW_ARR(ElToken, new_cap);

    el_tkque_repack(tkque->data, tkque->cap, tkque->head, tkque->tail, tkque->len, new_data);

    el_free(tkque->data);
    tkque->data = new_data;
    tkque->cap = new_cap;
    tkque->head = 0;
    tkque->tail = tkque->len;
}

void el_tkque_init(ElTokenQueue* tkque) {
    el_tkque_init_with_cap(tkque, EL_TKQUE_DEFAULT_CAP);
}

void el_tkque_init_with_cap(ElTokenQueue* tkque, usize initial_cap) {
    memset(tkque, 0, sizeof(*tkque));
    if (initial_cap <= 0) {
        initial_cap = EL_TKQUE_DEFAULT_CAP;
    }

    tkque->data = EL_NEW_ARR(ElToken, initial_cap);
    tkque->cap = initial_cap;
}

void el_tkque_destroy(ElTokenQueue* tkque) {
    el_free(tkque->data);
    memset(tkque, 0, sizeof(*tkque));
}

void el_tkque_copy(const ElTokenQueue* src, ElTokenQueue* dst) {
    dst->data = EL_NEW_ARR(ElToken, src->cap);

    el_tkque_repack(src->data, src->cap, src->head, src->tail, src->len, dst->data);

    dst->cap = src->cap;
    dst->len = src->len;
    dst->head = src->head;
    dst->tail = src->tail;
}

void el_tkque_move(ElTokenQueue* src, ElTokenQueue* dst) {
    *dst = *src;
    memset(src, 0, sizeof(*src));
}

void el_tkque_push(ElTokenQueue* tkque, ElToken tok) {
    if (tkque->len == tkque->cap) {
        el_tkque_grow(tkque);
    }

    tkque->data[tkque->tail] = tok;
    tkque->tail = (tkque->tail + 1) % tkque->cap;
    tkque->len++;
}

void el_tkque_push_front(ElTokenQueue* tkque, ElToken tok) {
    if (tkque->len == tkque->cap) {
        el_tkque_grow(tkque);
    }

    tkque->head = (tkque->head + tkque->cap - 1) % tkque->cap;
    tkque->data[tkque->head] = tok;
    tkque->len++;
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

    if (out_tok != NULL) {
        *out_tok = tkque->data[(tkque->head + index) % tkque->cap];
    }
    return true;
}

void el_tkque_clear(ElTokenQueue* tkque) {
    tkque->head = 0;
    tkque->tail = 0;
    tkque->len = 0;
}

void el_tkque_reserve(ElTokenQueue* tkque, usize min_cap) {
    if (tkque->cap >= min_cap) {
        return;
    }

    usize new_cap = tkque->cap == 0 ? EL_TKQUE_DEFAULT_CAP : tkque->cap;
    while (new_cap < min_cap) {
        new_cap *= 2;
    }

    ElToken* new_data = EL_NEW_ARR(ElToken, new_cap);

    el_tkque_repack(tkque->data, tkque->cap, tkque->head, tkque->tail, tkque->len, new_data);

    el_free(tkque->data);
    tkque->data = new_data;
    tkque->cap = new_cap;
    tkque->head = 0;
    tkque->tail = tkque->len;
}
