#pragma once

#include <elash/lexer/token.h>
#include <elash/lexer/tokstream.h>

typedef struct ElTokenBuf {
    ElToken* data;
    usize len;
    usize cap;
} ElTokenBuf;

typedef struct ElTkBufStream {
    const ElTokenBuf* buf;
    usize pos;
} ElTkBufStream;

void el_tkbuf_init(ElTokenBuf* tkbuf);
void el_tkbuf_destroy(ElTokenBuf* tkbuf);

void el_tkbuf_copy(const ElTokenBuf* src, ElTokenBuf* dst);
void el_tkbuf_move(ElTokenBuf* src, ElTokenBuf* dst);

void el_tkbuf_resize(ElTokenBuf* tkbuf, usize new_size);
void el_tkbuf_reserve(ElTokenBuf* tkbuf, usize min_cap);
void el_tkbuf_reserve_exact(ElTokenBuf* tkbuf, usize new_cap);

void el_tkbuf_push(ElTokenBuf* tkbuf, ElToken tok);

void el_tkbuf_clear(ElTokenBuf* tkbuf);

void el_tkbuf_capture_stream(ElTokenStream* stream, ElTokenBuf* buf, ElDiagEngine* diag);
ElTokenStream el_tkbuf_as_stream(ElTkBufStream* ctx, const ElTokenBuf* buf);
