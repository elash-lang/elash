#include "pp-test-utils.h"

TestSuite(el_pp_unhappy, .init = init, .fini = fini);

Test(el_pp_unhappy, stray_else) {
    ElPreproc* pp = p(
        "#else"
        "   #error 'W', 't', 'f'"
        "#end"
    );
    assert_has_err(pp, "pp.stray-else");
}

Test(el_pp_unhappy, invalid_op) {
    ElPreproc* pp = p(
        "#const foo = \"Hello\" * 2 / 'X'"
    );
    assert_has_err(pp, "pp.invalid-op");
}

Test(el_pp_unhappy, duplicated_else) {
    ElPreproc* pp = p(
        "#var thing = 30\n"
        "#if thing >= 20\n"
        "#else\n"
        "#else\n"
        "#end\n"
    );
    assert_has_err(pp, "pp.dup-else");
}

Test(el_pp_unhappy, func_no_return) {
    ElPreproc* pp = p(
        "#func thing()\n"
        "    // no #return\n"
        "#end\n"
        "#const x = thing()\n"
    );
    assert_has_err(pp, "pp.func-no-return");
}

Test(el_pp_unhappy, mutate_const) {
    ElPreproc* pp = p(
        "#const x = 311\n"
        "#set x = 99\n"
    );
    assert_has_err(pp, "pp.immutable");
}

Test(el_pp_unhappy, promote_redef) {
    ElPreproc* pp = p(
        "#const x = 1\n"
        "#if true\n"
        "    #const x = 2\n"
        "#end\n"
    );
    assert_has_err(pp, "pp.redefinition");
}

Test(el_pp_unhappy, func_token_dirs) {
    ElPreproc* pp = p(
        "#func invalid()\n"
        "    #emit Foo bar = baz\n"
        "#end\n"
    );
    assert_has_err(pp, "pp.func-emit");
}

Test(el_pp_unhappy, elif_after_else) {
    ElPreproc* pp = p(
        "#if true\n"
        "#else\n"
        "#elif true\n"
        "#end\n"
    );
    assert_has_err(pp, "pp.elif-after-else");
}

Test(el_pp_unhappy, cond_type) {
    ElPreproc* pp = p(
        "#var i = 3.14"
        "#while i - 1"
        "   #error \"should not be reached\""
        "#end"
    );
    assert_has_err(pp, "pp.cond-type");
}
