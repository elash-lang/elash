#include <elash/lexer/tokbuf.h>
#include <elash/util/alloc.h>

#include <string.h>

static void el_tkbuf_reallocate(ElTokenBuf* tkbuf, usize new_cap);

static ElToken el_tkbuf_stream_next(ElTokenStream* self, ElDiagEngine* engine) {
    (void) engine;
    ElTkBufStream* ctx = self->ctx;
    if (ctx->pos >= ctx->buf->len)
        return (ElToken) { .type = EL_TT_EOF };
    return ctx->buf->data[ctx->pos++];
}

ElTokenStream el_tkbuf_as_stream(ElTkBufStream* ctx, const ElTokenBuf* buf) {
    ctx->buf = buf;
    ctx->pos = 0;
    return (ElTokenStream) {
        .next = el_tkbuf_stream_next,
        .ctx = ctx,
    };
}

void el_tkbuf_init(ElTokenBuf* tkbuf) {
    tkbuf->data = NULL;
    tkbuf->len = 0;
    tkbuf->cap = 0;
}

void el_tkbuf_destroy(ElTokenBuf* tkbuf) {
    el_free(tkbuf->data);
    tkbuf->data = NULL;
    tkbuf->len = 0;
    tkbuf->cap = 0;
}

void el_tkbuf_copy(const ElTokenBuf* src, ElTokenBuf* dst) {
    el_tkbuf_init(dst);

    if (src->len == 0)
        return;

    dst->data = EL_NEW_ARR(ElToken, src->len);

    memcpy(dst->data, src->data, src->len * sizeof(ElToken));
    dst->len = src->len;
    dst->cap = src->len;
}

void el_tkbuf_move(ElTokenBuf* src, ElTokenBuf* dst) {
    el_tkbuf_destroy(dst);

    dst->data = src->data;
    dst->len = src->len;
    dst->cap = src->cap;

    el_tkbuf_init(src);
}

static void el_tkbuf_reallocate(ElTokenBuf* tkbuf, usize new_cap) {
    if (new_cap == tkbuf->cap) return;

    tkbuf->data = el_realloc(tkbuf->data, new_cap, sizeof(ElToken));
    tkbuf->cap = new_cap;
}

void el_tkbuf_reserve(ElTokenBuf* tkbuf, usize min_cap) {
    if (tkbuf->cap >= min_cap) {
        return;
    }

    // NOLINTNEXTLINE
    usize new_cap = tkbuf->cap == 0 ? (min_cap < 4 ? 4 : min_cap) : tkbuf->cap * 2;
    if (new_cap < min_cap) {
        new_cap = min_cap;
    }

    el_tkbuf_reallocate(tkbuf, new_cap);
}

void el_tkbuf_reserve_exact(ElTokenBuf* tkbuf, usize new_cap) {
    if (tkbuf->cap == new_cap) return;

    el_tkbuf_reallocate(tkbuf, new_cap);
}

void el_tkbuf_resize(ElTokenBuf* tkbuf, usize new_size) {
    if (new_size > tkbuf->cap) {
        el_tkbuf_reserve_exact(tkbuf, new_size);
    }

    if (new_size > tkbuf->len) {
        memset(
            tkbuf->data + tkbuf->len,
            0,
            (new_size - tkbuf->len) * sizeof(ElToken)
        );
    }

    tkbuf->len = new_size;
}

void el_tkbuf_push(ElTokenBuf* tkbuf, ElToken tok) {
    if (tkbuf->len == tkbuf->cap) {
        el_tkbuf_reserve(tkbuf, tkbuf->len + 1);
    }

    tkbuf->data[tkbuf->len++] = tok;
}

void el_tkbuf_capture_stream(ElTokenStream* stream, ElTokenBuf* buf, ElDiagEngine* diag) {
    ElToken tok;
    do {
        tok = stream->next(stream, diag);
        if (tok.type != EL_TT_EOF) {
            el_tkbuf_push(buf, tok);
        }
    } while (tok.type != EL_TT_EOF);
}

void el_tkbuf_clear(ElTokenBuf* tkbuf) {
    tkbuf->len = 0;
}
