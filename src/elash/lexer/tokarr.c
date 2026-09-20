#include <elash/lexer/tokarr.h>

static ElToken _el_token_array_stream_next(ElTokenStream* self, ElDiagEngine* engine) {
    (void) engine;
    ElTokenArrayStream* ctx = self->ctx;
    if (ctx->pos >= ctx->count)
        return (ElToken) { .type = EL_TT_EOF };
    return ctx->data[ctx->pos++];
}

ElTokenStream el_tokarr_as_stream(ElTokenArrayStream* ctx, ElTokenArray array) {
    return el_new_tokarr_as_stream(ctx, array.data, array.count);
}

ElTokenStream el_new_tokarr_as_stream(ElTokenArrayStream* ctx, const ElToken* data, usize count) {
    ctx->data = data;
    ctx->count = count;
    ctx->pos = 0;

    return (ElTokenStream) {
        .next = _el_token_array_stream_next,
        .ctx = ctx,
    };
}
