#pragma once

#include <elash/util/dynarena.h>

#include <elash/lexer/tokbuf.h>
#include <elash/lexer/tokque.h>
#include <elash/lexer/token.h>
#include <elash/lexer/tokstream.h>

#include <elash/pp/include.h>
#include <elash/pp/scope.h>

#include <elash/source/doc.h>

#include <stdbool.h>

typedef struct ElPpFrame ElPpFrame;
typedef struct ElPpIfFrame ElPpIfFrame;

typedef struct ElPreproc {
    uint include_depth;
    ElPpFrame* frame;

    uint skip_depth;
    ElPpIfFrame* if_stack;

    ElTokenQueue pending;

    ElToken lookahead;
    bool has_lookahead;

    const ElPpIncMap* imap;

    ElDynArena* farena;
    ElDynArena* iarena;

    ElDiagEngine* diag;

    ElPpScope* builtin_scope;
    ElPpScope* global_scope;
    ElPpScope* current_scope;
} ElPreproc;

bool el_pp_init(
    ElPreproc* pp, ElTokenStream input, const ElSourceDocument* root_doc,
    ElDynArena* arena, const ElPpIncMap* imap
);
void el_pp_free(ElPreproc* pp);

bool el_pp_next(ElPreproc* pp, ElToken* out_tok, ElDiagEngine* diag);

ElTokenStream el_pp_as_token_stream(ElPreproc* pp);
