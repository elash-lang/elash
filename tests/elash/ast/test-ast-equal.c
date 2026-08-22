#include <criterion/criterion.h>

#include <elash/ast/equal.h>
#include <elash/lexer/lexer.h>
#include <elash/util/dynarena.h>
#include <elash/parser/parser.h>

static ElSourceSpan span = EL_SRCSPAN_NULL;

static ElDynArena arena;
void init() { el_dynarena_init(&arena); }
void fini() { el_dynarena_free(&arena); }

TestSuite(ast_equal, .init = init, .fini = fini);
#define LEX_FLAGS (EL_LF_SKIP_WHITESPACE | EL_LF_SKIP_COMMENTS)

// multi-line strings in C. yes. this is real.
#define S(...) EL_SV(#__VA_ARGS__)

/// --- integration test ---
Test(ast_equal, integration_test) {
    ElDiagEngine diag = {0};
    ElParser parser;
    ElLexer lexer;

    ElSourceDocument doc1, doc2;
    el_srcdoc_init_from_str(&doc1, S(
        int add(int a, int b) {
            return a + b;
        }
        int main() {
            struct(int, int) results = {
                add(10, 20),
                add(100, 200),
            };
            return results.0;
        }
    ), EL_SV("<test:doc1>"));
    el_srcdoc_init_from_str(&doc2, S(
        int sub(int a, int b) {
            return a - b;
        }
        int main() {
            struct(int, int) results = {
                add(10, 20),
                add(100, 200),
            };
            return results.0;
        }
    ), EL_SV("<test:doc2>"));

    el_lexer_init(&lexer, &doc1, LEX_FLAGS);
    el_parser_init(&parser, el_lexer_as_token_stream(&lexer), &diag, &arena);
    ElAstModule* mod1 = el_parser_parse_module(&parser);
    cr_assert(diag.summary.total_errors == 0);
    el_parser_destroy(&parser);

    el_lexer_init(&lexer, &doc2, LEX_FLAGS);
    el_parser_init(&parser, el_lexer_as_token_stream(&lexer), &diag, &arena);
    ElAstModule* mod2 = el_parser_parse_module(&parser);
    cr_assert(diag.summary.total_errors == 0);
    el_parser_destroy(&parser);

    cr_assert(el_ast_equal_module(mod1, mod1));
    cr_assert(!el_ast_equal_module(mod1, mod2));
    cr_assert(!el_ast_equal_module(mod2, mod1));
}

/// --- normal cases ---
Test(ast_equal, literals) {
    // int
    cr_assert(el_ast_equal_expr(el_ast_new_int_literal(&arena, span, 0), el_ast_new_int_literal(&arena, span, 0)));
    cr_assert(!el_ast_equal_expr(el_ast_new_int_literal(&arena, span, 0), el_ast_new_int_literal(&arena, span, 1)));

    // string
    cr_assert(el_ast_equal_expr(el_ast_new_string_literal(&arena, span, el_sv_from_cstr("a")), el_ast_new_string_literal(&arena, span, el_sv_from_cstr("a"))));
    cr_assert(!el_ast_equal_expr(el_ast_new_string_literal(&arena, span, el_sv_from_cstr("a")), el_ast_new_string_literal(&arena, span, el_sv_from_cstr("b"))));
}

Test(ast_equal, nulls) {
    ElAstExpr* stuff = el_ast_new_string_literal(&arena, span, EL_SV("tuff"));

    cr_assert(el_ast_equal_stmt(NULL, NULL));
    cr_assert(!el_ast_equal_expr(NULL, stuff));
}

Test(ast_equal, expressions) {
    // (1 + 2) * 3
    ElAstExpr* one = el_ast_new_int_literal(&arena, span, 1);
    ElAstExpr* two = el_ast_new_int_literal(&arena, span, 2);
    ElAstExpr* three = el_ast_new_int_literal(&arena, span, 3);

    ElAstExpr* plus = el_ast_new_bin_expr(&arena, span, EL_SEMA_BIN_OP_ADD, one, two);
    ElAstExpr* mul = el_ast_new_bin_expr(&arena, span, EL_SEMA_BIN_OP_MUL, plus, three);

    ElAstExpr* plus2 = el_ast_new_bin_expr(&arena, span, EL_SEMA_BIN_OP_ADD, el_ast_new_int_literal(&arena, span, 1), el_ast_new_int_literal(&arena, span, 2));
    ElAstExpr* mul2 = el_ast_new_bin_expr(&arena, span, EL_SEMA_BIN_OP_MUL, plus2, el_ast_new_int_literal(&arena, span, 3));

    cr_assert(el_ast_equal_expr(mul, mul2));

    // different op
    ElAstExpr* mul3 = el_ast_new_bin_expr(&arena, span, EL_SEMA_BIN_OP_SUB, plus, three);
    cr_assert(!el_ast_equal_expr(mul, mul3));
}

Test(ast_equal, declarations) {
    ElAstType* type = el_ast_new_type_name(&arena, span, el_ast_new_ident_raw(&arena, span, el_sv_from_cstr("int")));
    ElAstIdent* name = el_ast_new_ident_raw(&arena, span, el_sv_from_cstr("x"));

    ElAstDecl* v1 = el_ast_new_var_def(&arena, span, type, name, NULL, false);
    ElAstDecl* v2 = el_ast_new_var_def(&arena, span, type, name, NULL, false);

    cr_assert(el_ast_equal_decl(v1, v2));

    // different static flag
    ElAstDecl* v3 = el_ast_new_var_def(&arena, span, type, name, NULL, true);
    cr_assert(!el_ast_equal_decl(v1, v3));
}
