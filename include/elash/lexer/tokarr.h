#pragma once

#include <elash/lexer/token.h>
#include <elash/lexer/tokstream.h>

typedef struct ElTokenArrayStream {
    const ElToken* data;
    usize len;
    usize pos;
} ElTokenArrayStream;

ElTokenStream el_token_array_as_stream(ElTokenArrayStream* ctx, const ElToken* data, usize len);
