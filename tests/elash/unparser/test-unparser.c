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
#include <math.h>
#include <glob.h>

static ElDynArena arena;
void init() { el_dynarena_init(&arena); }
void fini() { el_dynarena_free(&arena); }

// NOLINTBEGIN(readability-magic-numbers)
Test(el_unparser, null_span_int_literal) {
    ElUnparser unparser;
    ElTokenBuf toks;
    el_tkbuf_init(&toks);
    el_unparser_init(&unparser, &toks, &arena);

    ElAstExpr* lit = el_ast_new_int_lit(&arena, EL_SRCSPAN_NULL, EL_INT128(42));

    el_unparser_unparse_expr(&unparser, lit);

    cr_assert_eq(toks.data[0].type, EL_TT_INT_LITERAL);
    cr_assert_str_eq(toks.data[0].lexeme.data, "42");
}

Test(el_unparser, null_span_float_literal) {
    ElUnparser unparser;
    ElTokenBuf toks;
    el_tkbuf_init(&toks);
    el_unparser_init(&unparser, &toks, &arena);

    const double expected = 3.14;
    ElAstExpr* lit = el_ast_new_float_lit(&arena, EL_SRCSPAN_NULL, expected);

    el_unparser_unparse_expr(&unparser, lit);

    double actual = strtod(toks.data[0].lexeme.data, NULL);
    cr_assert_eq(toks.data[0].type, EL_TT_FLOAT_LITERAL);
    cr_assert(fabs(actual - expected) < 1e-9, "Expected %f, got %s", expected, toks.data[0].lexeme.data);
}

// NOLINTEND(readability-magic-numbers)
