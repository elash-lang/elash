#pragma once

#include <elash/lexer/token.h>
#include <elash/defs/int-types.h>

#include <stdbool.h>

typedef struct ElTokenQueue {
    ElToken* data;
    usize cap;
    usize head;
    usize tail;
    usize len;
} ElTokenQueue;

bool el_tkque_init(ElTokenQueue* tkque);
bool el_tkque_init_with_cap(ElTokenQueue* tkque, usize initial_cap);
void el_tkque_destroy(ElTokenQueue* tkque);

bool el_tkque_copy(const ElTokenQueue* src, ElTokenQueue* dst);
void el_tkque_move(ElTokenQueue* src, ElTokenQueue* dst);

bool el_tkque_push(ElTokenQueue* tkque, ElToken tok);
bool el_tkque_pop(ElTokenQueue* tkque, ElToken* out_tok);

bool el_tkque_peek(const ElTokenQueue* tkque, ElToken* out_tok);
bool el_tkque_at(const ElTokenQueue* tkque, usize index, ElToken* out_tok);
bool el_tkque_clear(ElTokenQueue* tkque);

bool el_tkque_reserve(ElTokenQueue* tkque, usize min_additional_cap);
