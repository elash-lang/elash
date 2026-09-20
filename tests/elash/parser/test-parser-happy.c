#include <criterion/criterion.h>
#include "parser-test-utils.h"

TestSuite(el_parser_happy, .init = init, .fini = fini);

Test(el_parser_happy, parse_expr) {
    ElDiagEngine diag;
    ElParser parser = p("1 + 2 / 30 - ('X' + \"Hello\")", &diag);
    ElAstExpr* expr = el_parser_parse_expr(&parser);

    cr_assert_eq(diag.summary.total_errors, 0);

    cr_assert_eq(expr->type, EL_AST_EXPR_BINARY);
    cr_assert_eq(expr->as.binary.op, EL_BIN_OP_SUB);

    // left: 1 + 2 / 30
    ElAstExpr* left = expr->as.binary.left;
    cr_assert_not_null(left);
    cr_assert_eq(left->type, EL_AST_EXPR_BINARY);
    cr_assert_eq(left->as.binary.op, EL_BIN_OP_ADD);

    // left-left: 1
    cr_assert_eq(left->as.binary.left->type, EL_AST_EXPR_LITERAL);
    assert_int_lit(left->as.binary.left, EL_INT128(1));

    // left-right: 2 / 30
    ElAstExpr* div_node = left->as.binary.right;
    cr_assert_not_null(div_node);
    cr_assert_eq(div_node->type, EL_AST_EXPR_BINARY);
    cr_assert_eq(div_node->as.binary.op, EL_BIN_OP_DIV);

    assert_int_lit(div_node->as.binary.left,  EL_INT128(2));
    assert_int_lit(div_node->as.binary.right, EL_INT128(30));

    // right: ('X' + "Hello")
    ElAstExpr* right = expr->as.binary.right;
    cr_assert_not_null(right);
    cr_assert_eq(right->type, EL_AST_EXPR_BINARY);
    cr_assert_eq(right->as.binary.op, EL_BIN_OP_ADD);
    assert_char_lit(right->as.binary.left, 'X');
    assert_str_lit(right->as.binary.right, "Hello");
}

Test(el_parser_happy, parse_stmt_println) {
    ElDiagEngine diag;
    ElParser parser = p("println(\"Hello, world!\");", &diag);
    ElAstStmt* stmt = el_parser_parse_stmt(&parser);
    cr_assert_eq(diag.summary.total_errors, 0);

    cr_assert_not_null(stmt);
    cr_assert_eq(stmt->type, EL_AST_STMT_EXPR);
    cr_assert_not_null(stmt->as.expr);

    ElAstExpr* expr = stmt->as.expr;
    cr_assert_eq(expr->type, EL_AST_EXPR_CALL);

    // callee: println
    ElAstExpr* callee = expr->as.call.callee;
    cr_assert_not_null(callee);
    cr_assert_eq(callee->type, EL_AST_EXPR_IDENT);
    cr_assert(el_sv_eql(callee->as.ident.name, EL_SV("println")));

    // args: "Hello, world!"
    cr_assert_eq(expr->as.call.arg_count, 1);
    ElAstToI* arg = expr->as.call.args;
    cr_assert_not_null(arg);
    cr_assert_eq(arg->kind, EL_AST_TOI_INIT);
    cr_assert_not_null(arg->as.init);
    cr_assert_eq(arg->as.init->kind, EL_AST_INIT_EXPR);
    cr_assert_not_null(arg->as.init->expr);
    cr_assert_eq(arg->as.init->expr->type, EL_AST_EXPR_LITERAL);
    assert_str_lit(arg->as.init->expr, "Hello, world!");
}

Test(el_parser_happy, parse_stmt_aug_assign) {
    ElDiagEngine diag;
    ElParser parser = p("x += 10;", &diag);
    ElAstStmt* stmt = el_parser_parse_stmt(&parser);
    cr_assert_eq(diag.summary.total_errors, 0);

    cr_assert_not_null(stmt);
    cr_assert_eq(stmt->type, EL_AST_STMT_COMPOUND_ASSIGN);

    // target: x
    cr_assert_not_null(stmt->as.cassign.target);
    cr_assert_eq(stmt->as.cassign.target->type, EL_AST_EXPR_IDENT);
    cr_assert(el_sv_eql(stmt->as.cassign.target->as.ident.name, EL_SV("x")));

    // op: +=
    cr_assert_eq(stmt->as.cassign.op, EL_BIN_OP_ADD);

    // value: 10
    cr_assert_not_null(stmt->as.cassign.value);
    cr_assert_eq(stmt->as.cassign.value->kind, EL_AST_INIT_EXPR);
    cr_assert_not_null(stmt->as.cassign.value->expr);
    cr_assert_eq(stmt->as.cassign.value->expr->type, EL_AST_EXPR_LITERAL);
    assert_int_lit(stmt->as.cassign.value->expr, EL_INT128(10));
}

Test(el_parser_happy, parse_decl_func) {
    ElDiagEngine diag;
    ElParser parser = p("int foo(int x);", &diag);
    ElAstDecl* decl = el_parser_parse_decl(&parser);
    cr_assert_eq(diag.summary.total_errors, 0);

    cr_assert_not_null(decl);
    cr_assert_eq(decl->type, EL_AST_DECL_FUNC_DECL);

    // signature: int foo(int x)
    ElAstFuncSignature sig = decl->as.func_decl.sig;
    cr_assert_not_null(sig.name);
    cr_assert(el_sv_eql(sig.name->name, EL_SV("foo")));

    // return type: int
    cr_assert_not_null(sig.ret_type);
    cr_assert_eq(sig.ret_type->kind, EL_AST_TYPE_NAME);
    cr_assert_not_null(sig.ret_type->as.name);
    cr_assert(el_sv_eql(sig.ret_type->as.name->name, EL_SV("int")));

    // params: (int x)
    ElAstFuncParam* param = sig.params.head;
    cr_assert_eq(sig.params.count, 1);
    cr_assert(el_sv_eql(param->name->name, EL_SV("x")));
    cr_assert_not_null(param->type);
    cr_assert_eq(param->type->kind, EL_AST_TYPE_NAME);
    cr_assert_not_null(param->type->as.name);
    cr_assert(el_sv_eql(param->type->as.name->name, EL_SV("int")));
}

