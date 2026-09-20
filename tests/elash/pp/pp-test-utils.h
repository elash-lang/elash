#pragma once

#include <criterion/criterion.h>

#include <elash/pp/preproc.h>
#include <elash/lexer/lexer.h>
#include <elash/source/doc.h>
#include <elash/diag/engine.h>
#include <elash/util/dynarena.h>

#include <stdlib.h>

static ElDynArena arena;
static ElDiagEngine diag;

void init() { el_dynarena_init(&arena); el_diag_engine_init(&diag, &arena); }
void fini() { el_dynarena_free(&arena); el_diag_engine_free(&diag);         }

// NOTE: memory leaks are intentional
static inline ElPreproc* p(const char* code) {
    ElSourceDocument* doc = malloc(sizeof *doc);
    el_srcdoc_init_from_str(doc, el_sv_from_cstr(code), EL_SV("test.eu"));

    ElLexer* lexer = malloc(sizeof *lexer);
    el_lexer_init(lexer, doc, EL_LEXER_FLAGS_DEFAULT);

    ElTokenStream stream = el_lexer_as_token_stream(lexer);

    static const ElPpIncMap empty_imap = {0};

    ElPreproc* pp = EL_DYNARENA_NEW(&arena, ElPreproc);
    bool ok = el_pp_init(pp, stream, doc, &arena, &empty_imap, NULL);
    cr_assert(ok, "el_pp_init failed");
    return pp;
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
static inline void assert_tok(
    ElPreproc* pp, ElTokenType expected_type, const char* expected_lexeme
) {
    ElToken tok;
    bool ok = el_pp_next(pp, &tok, &diag);

    ElStringView expected_type_sv = el_token_type_to_string(expected_type);
    cr_assert(ok, "expected token " EL_SV_FMT " but got EOF", EL_SV_FARG(expected_type_sv));

    ElStringView actual_type_sv = el_token_type_to_string(tok.type);
    cr_assert_eq(tok.type, expected_type,
                 "expected " EL_SV_FMT ", got " EL_SV_FMT " (lexeme: '" EL_SV_FMT "')",
                 EL_SV_FARG(expected_type_sv), EL_SV_FARG(actual_type_sv), EL_SV_FARG(tok.lexeme));

    if (expected_lexeme) {
        ElStringView expected_sv = el_sv_from_cstr(expected_lexeme);
        cr_assert(el_sv_eql(tok.lexeme, expected_sv),
                  "expected lexeme '" EL_SV_FMT "', got '" EL_SV_FMT "'",
                  EL_SV_FARG(expected_sv), EL_SV_FARG(tok.lexeme));
    }
}

static inline void assert_eof(ElPreproc* pp) {
    ElToken tok;
    bool ok = el_pp_next(pp, &tok, &diag);
    ElStringView actual_sv = el_token_type_to_string(tok.type);
    cr_assert(!ok, "expected EOF but got " EL_SV_FMT " ('" EL_SV_FMT "')",
              EL_SV_FARG(actual_sv), EL_SV_FARG(tok.lexeme));
}

static inline void assert_no_errors() {
    cr_assert_eq(diag.summary.total_errors, 0u,
                 "expected 0 errors, got %u", diag.summary.total_errors);
}

static inline void assert_has_err(ElPreproc* pp, const char* expected_category) {
    while (el_pp_next(pp, NULL, &diag));

    ElStringView expected_sv = el_sv_from_cstr(expected_category);
    bool found = false;

    for (ElDiagnostic* d = diag.diag.head; d != NULL; d = d->next) {
        if (el_sv_eql(d->category, expected_sv) && d->sev == EL_DIAG_ERROR) {
            found = true;
            break;
        }
    }

    cr_assert(found, "expected error category '" EL_SV_FMT "', but not found", EL_SV_FARG(expected_sv));
}
