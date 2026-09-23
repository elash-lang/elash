#include <criterion/criterion.h>
#include "parser-test-utils.h"

TestSuite(el_parser_unhappy, .init = init, .fini = fini);

Test(el_parser_unhappy, parse_invalid_expr) {
    ElDiagEngine diag;
    ElParser parser = p("1 + ;", &diag);
    (void)el_parse_expr(&parser);
    cr_assert_gt(diag.summary.total_errors, 0);
}

Test(el_parser_unhappy, parse_invalid_stmt) {
    ElDiagEngine diag;
    ElParser parser = p("x +=", &diag);
    (void)el_parse_stmt(&parser);
    cr_assert_gt(diag.summary.total_errors, 0);
}

Test(el_parser_unhappy, parse_invalid_decl) {
    ElDiagEngine diag;
    ElParser parser = p("int foo(int x", &diag);
    (void)el_parse_decl(&parser);
    cr_assert_gt(diag.summary.total_errors, 0);
}

Test(el_parser_unhappy, parse_type_without_name) {
    ElDiagEngine diag;
    ElParser parser = p("int; float;\nchar 123 + 3551;", &diag);
    (void)el_parse_module(&parser);
    cr_assert_gt(diag.summary.total_errors, 0);
}
