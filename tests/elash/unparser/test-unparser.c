#include <criterion/criterion.h>
#include <elash/defs/int-types.h>

#include <elash/ast/equal.h>
#include <elash/util/ansi.h>
#include <elash/lexer/lexer.h>
#include <elash/lexer/tokbuf.h>
#include <elash/util/dynarena.h>
#include <elash/parser/parser.h>
#include <elash/unparser/unparser.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <glob.h>

static ElDynArena arena;
void init() { el_dynarena_init(&arena); }
void fini() { el_dynarena_free(&arena); }

void print_run_info(const char* path) {
    el_ansi_pref = EL_ANSI_AUTO;

    ElAnsiStyle style1 = {
        .fg_color = EL_ANSI_CLR_MAGENTA,
        .dec = EL_ANSI_DEC_BOLD,
        .bg_color = EL_ANSI_CLR_DEFAULT,
    };

    ElAnsiStyle style2 = {
        .fg_color = EL_ANSI_CLR_DEFAULT,
        .dec = EL_ANSI_DEC_BOLD,
        .bg_color = EL_ANSI_CLR_DEFAULT,
    };

    fputs("[", stdout);
    el_ansi_apply_style(style1, stdout);
    fputs("++++", stdout);
    el_ansi_reset_style(stdout);
    fputs("] ", stdout);
    el_ansi_apply_style(style2, stdout);
    fputs("Unparsing: ", stdout);
    el_ansi_reset_style(stdout);
    puts(path);
}

TestSuite(el_unparser, .init = init, .fini = fini);

// --- integration test ---
Test(el_unparser, integration_test) {
    // this kinda sucks but i guess will work for now
    glob_t g;
    cr_assert_eq(glob("tests/e2e/positive/*/*.eu", 0, NULL, &g), 0);
    cr_assert_eq(glob("tests/e2e/positive/*/*/*.eu", GLOB_APPEND, NULL, &g), 0);

    ElDiagEngine diag = {0};
    ElTkBufStream stream;
    ElTokenBuf toks;

    ElUnparser unparser;
    ElParser parser;
    ElLexer lexer;

    el_tkbuf_init(&toks);
    for (usize i = 0; i < g.gl_pathc; ++i) {
        // very sloppy approach but works
        if (strstr(g.gl_pathv[i], "/preproc/") != NULL)
            continue;
        print_run_info(g.gl_pathv[i]);

        el_tkbuf_clear(&toks);

        ElSourceDocument doc;
        cr_assert_eq(el_srcdoc_init_from_file(&doc, g.gl_pathv[i]), EL_SRCDOC_ERR_SUCCESS);

        el_lexer_init(&lexer, &doc, EL_LEXER_FLAGS_DEFAULT|EL_LF_SKIP_WHITESPACE|EL_LF_SKIP_COMMENTS);

        el_parser_init(&parser, el_lexer_as_token_stream(&lexer), &diag, &arena);
        ElAstModule* orig = el_parser_parse_module(&parser);
        el_parser_destroy(&parser);

        el_unparser_init(&unparser, &toks, &arena);
        el_unparser_unparse_module(&unparser, orig);

        el_parser_init(&parser, el_tkbuf_as_stream(&stream, &toks), &diag, &arena);
        ElAstModule* repro = el_parser_parse_module(&parser);
        el_parser_destroy(&parser);

        cr_assert(el_ast_equal_module(orig, repro));
    }

    globfree(&g);
}

// --- normal cases ---
// NOLINTBEGIN(readability-magic-numbers)
Test(el_unparser, null_span_int_literal) {
    ElUnparser unparser;
    ElTokenBuf toks;
    el_tkbuf_init(&toks);
    el_unparser_init(&unparser, &toks, &arena);

    ElAstExpr* lit = el_ast_new_int_literal(&arena, EL_SRCSPAN_NULL, 42);

    bool success = el_unparser_unparse_expr(&unparser, lit);
    cr_assert(success);

    cr_assert_eq(toks.data[0].type, EL_TT_INT_LITERAL);
    cr_assert_str_eq(toks.data[0].lexeme.data, "42");
}

Test(el_unparser, null_span_float_literal) {
    ElUnparser unparser;
    ElTokenBuf toks;
    el_tkbuf_init(&toks);
    el_unparser_init(&unparser, &toks, &arena);

    const double expected = 3.14;
    ElAstExpr* lit = el_ast_new_float_literal(&arena, EL_SRCSPAN_NULL, expected);

    bool success = el_unparser_unparse_expr(&unparser, lit);
    cr_assert(success);

    double actual = atof(toks.data[0].lexeme.data);
    cr_assert_eq(toks.data[0].type, EL_TT_FLOAT_LITERAL);
    cr_assert(fabs(actual - expected) < 1e-9, "Expected %f, got %s", expected, toks.data[0].lexeme.data);
}

// NOLINTEND(readability-magic-numbers)
