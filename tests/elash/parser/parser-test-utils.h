#include <elash/parser/parser.h>
#include <elash/lexer/lexer.h>
#include <elash/source/doc.h>

#include <stdlib.h>

#define assert_int_lit(LIT, EXPECTED) do {                   \
    cr_assert_eq((LIT)->as.literal.type, EL_AST_LIT_INT);    \
    cr_assert_eq((LIT)->as.literal.of.int_.value, EXPECTED); \
} while (0)

#define assert_char_lit(LIT, EXPECTED) do {                   \
    cr_assert_eq((LIT)->as.literal.type, EL_AST_LIT_CHAR);    \
    cr_assert_eq((LIT)->as.literal.of.char_.value, EXPECTED); \
} while (0)

#define assert_str_lit(LIT, EXPECTED) do {                                  \
    cr_assert_eq((LIT)->as.literal.type, EL_AST_LIT_STRING);                \
    cr_assert(el_sv_eql((LIT)->as.literal.of.str_.value, EL_SV(EXPECTED))); \
} while (0)

static ElDynArena arena;
void init() { el_dynarena_init(&arena); }
void fini() { el_dynarena_free(&arena); }

// NOTE: memory leaks are intentional
ElParser p(const char* code, ElDiagEngine* diag) {
    ElSourceDocument* doc = malloc(sizeof *doc);
    el_srcdoc_init_from_str(doc, el_sv_from_cstr(code), EL_SV("test.eu"));

    ElLexer* lexer = malloc(sizeof *lexer);
    el_lexer_init(lexer, doc, EL_LEXER_FLAGS_DEFAULT | EL_LF_SKIP_WHITESPACE);

    ElTokenStream toks = el_lexer_as_token_stream(lexer);
    el_diag_engine_init(diag, &arena);

    ElParser parser;
    el_parser_init(&parser, toks, diag, &arena);
    return parser;
}

