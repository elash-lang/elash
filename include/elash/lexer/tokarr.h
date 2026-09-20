#pragma once

#include <elash/lexer/token.h>
#include <elash/lexer/tokstream.h>

#define EL_TOKARR_NULL \
    ((ElTokenArray) { .data = NULL, .count = 0 })

typedef struct ElTokenArray {
    ElToken* data;
    usize count;
} ElTokenArray;

typedef struct ElTokenArrayStream {
    const ElToken* data;
    usize count;
    usize pos;
} ElTokenArrayStream;

ElTokenStream el_tokarr_as_stream(ElTokenArrayStream* ctx, ElTokenArray array);
ElTokenStream el_new_tokarr_as_stream(ElTokenArrayStream* ctx, const ElToken* data, usize count);
