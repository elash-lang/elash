#include "pp-test-utils.h"

TestSuite(el_pp_happy, .init = init, .fini = fini);

Test(el_pp_happy, emit) {
    ElPreproc* pp = p(
        "\"normal\"\n"
        "#emit \"emitted\"\n"
        "\"normal\"\n"
        "#emit #{1 + 3 - 2}\n"
    );

    assert_tok(pp, EL_TT_STRING_LITERAL, "normal");
    assert_tok(pp, EL_TT_STRING_LITERAL, "emitted");
    assert_tok(pp, EL_TT_STRING_LITERAL, "normal");
    assert_tok(pp, EL_TT_INT_LITERAL,    "2");
    assert_eof(pp);
    assert_no_errors();
}

Test(el_pp_happy, conditional) {
    ElPreproc* pp = p(
        "#const cond = 1 == 1"
        "#if cond"
        "    foo\n"
        "    #emit 1\n"
        "#else"
        "   #emit bar\n"
        "#end"
    );

    assert_tok(pp, EL_TT_IDENT,       "foo");
    assert_tok(pp, EL_TT_INT_LITERAL, "1");
    assert_eof(pp);
    assert_no_errors();
}

Test(el_pp_happy, functions) {
    ElPreproc* pp = p(
        "#func f(n)"
        "   #if n <= 1"
        "       #return 1"
        "   #end"
        "   #return n * f(n - 1)"
        "#end"
        "#emit #{{f(1), f(2), f(3), f(4), f(5)}}"
    );

    assert_tok(pp, EL_TT_INT_LITERAL, "1");
    assert_tok(pp, EL_TT_INT_LITERAL, "2");
    assert_tok(pp, EL_TT_INT_LITERAL, "6");
    assert_tok(pp, EL_TT_INT_LITERAL, "24");
    assert_tok(pp, EL_TT_INT_LITERAL, "120");
    assert_eof(pp);
    assert_no_errors();
}

Test(el_pp_happy, variables) {
    ElPreproc* pp = p(
        "#var x = 1"
        "#set x = x + 1"
        "#emit #{x}"
    );

    assert_tok(pp, EL_TT_INT_LITERAL, "2");
    assert_eof(pp);
    assert_no_errors();
}

Test(el_pp_happy, scopes) {
    ElPreproc* pp = p(
        "#var x = 1\n"
        "#if true\n"
        "    #var internal x = 2\n"
        "    #emit #{x}\n"
        "#end\n"
        "#emit #{x}\n"
    );

    assert_tok(pp, EL_TT_INT_LITERAL, "2");
    assert_tok(pp, EL_TT_INT_LITERAL, "1");
    assert_eof(pp);
    assert_no_errors();
}

Test(el_pp_happy, while_loops) {
    ElPreproc* pp = p(
        "#var i = 0\n"
        "#while i < 5\n"
        "   #emit #{i}\n"
        "   #inc i\n"
        "#end\n"
    );

    assert_tok(pp, EL_TT_INT_LITERAL, "0");
    assert_tok(pp, EL_TT_INT_LITERAL, "1");
    assert_tok(pp, EL_TT_INT_LITERAL, "2");
    assert_tok(pp, EL_TT_INT_LITERAL, "3");
    assert_tok(pp, EL_TT_INT_LITERAL, "4");
    assert_eof(pp);
    assert_no_errors();
}
