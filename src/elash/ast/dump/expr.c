#include <elash/ast/dump/expr.h>
#include <elash/ast/tree/expr.h>

#include <elash/ast/dump/indent.h>
#include <elash/ast/dump/type.h>
#include <elash/ast/dump/init.h>
#include <inttypes.h>

void el_ast_dump_expr_literal(ElAstLiteral* lit, usize indent, FILE* out) {
    el_ast_dump_print_indent(indent, out);
    switch (lit->type) {
    case EL_AST_LIT_BOOL:
        fprintf(out, "BoolLiteral(%s)\n", lit->of.bool_.value ? "true" : "false");
        break;
    case EL_AST_LIT_NULL:
        fprintf(out, "NullLiteral\n");
        break;
    case EL_AST_LIT_CHAR:
        fprintf(out, "CharLiteral(%c)\n", lit->of.char_.value);
        break;
    case EL_AST_LIT_INT:
        fprintf(out, "IntLiteral(%"PRId64")\n", lit->of.int_.value);
        break;
    case EL_AST_LIT_FLOAT:
        fprintf(out, "FloatLiteral(%Lf)\n", lit->of.float_.value);
        break;
    case EL_AST_LIT_STRING:
        fprintf(out, "StringLiteral(\""EL_SV_FMT"\")\n", EL_SV_FARG(lit->of.str_.value));
        break;
    }
}

void el_ast_dump_expr_ident(ElAstIdent* node, usize indent, FILE* out) {
    el_ast_dump_print_indent(indent, out);
    fprintf(out, "IdentExpr(\""EL_SV_FMT"\")\n", EL_SV_FARG(node->name));
}

void el_ast_dump_expr(ElAstExpr* node, usize indent, FILE* out) {
    switch (node->type) {
    case EL_AST_EXPR_BINARY: {
        ElStringView op = el_sema_bin_op_to_string(node->as.binary.op);
        el_ast_dump_print_indent(indent, out);
        fprintf(out, "BinaryExpr('"EL_SV_FMT"'):\n", EL_SV_FARG(op));
        el_ast_dump_print_indent(indent + 1, out);
        fprintf(out, "left: \n"); el_ast_dump_expr(node->as.binary.left,  indent + 2, out);
        el_ast_dump_print_indent(indent + 1, out);
        fprintf(out, "right: \n"); el_ast_dump_expr(node->as.binary.right, indent + 2, out);
        break;
    }
    case EL_AST_EXPR_UNARY: {
        ElStringView op = el_sema_unary_op_format(node->as.unary.op);
        el_ast_dump_print_indent(indent, out);
        fprintf(out, "UnaryExpr('"EL_SV_FMT"'):\n", EL_SV_FARG(op));
        el_ast_dump_expr(node->as.unary.operand, indent + 1, out);
        break;
    }
    case EL_AST_EXPR_LITERAL:
        return el_ast_dump_expr_literal(&node->as.literal, indent, out);
    case EL_AST_EXPR_IDENT:
        return el_ast_dump_expr_ident(&node->as.ident, indent, out);
    case EL_AST_EXPR_CALL: {
        el_ast_dump_print_indent(indent, out);
        fprintf(out, "CallExpr(args=%zu):\n", node->as.call.arg_count);
        el_ast_dump_print_indent(indent + 1, out);
        fprintf(out, "callee:\n");
        el_ast_dump_expr(node->as.call.callee, indent + 2, out);
        el_ast_dump_print_indent(indent + 1, out);
        fprintf(out, "args:\n");
        for (ElAstInit* arg = node->as.call.args; arg != NULL; arg = arg->next) {
            el_ast_dump_init(arg, indent + 2, out);
        }
        break;
    }
    case EL_AST_EXPR_ARRAYLIT:
        el_ast_dump_print_indent(indent, out);
        fprintf(out, "ArrayLit:\n");
        el_ast_dump_print_indent(indent + 1, out);
        fprintf(out, "type:\n");
        el_ast_dump_type(node->as.array_lit.type, indent + 2, out);
        el_ast_dump_print_indent(indent + 1, out);
        fprintf(out, "init:\n");
        el_ast_dump_init(node->as.array_lit.init, indent + 2, out);
        break;
    case EL_AST_EXPR_CAST:
        el_ast_dump_print_indent(indent, out);
        fprintf(out, "CastExpr:\n");
        el_ast_dump_print_indent(indent + 1, out);
        fprintf(out, "expr:\n");
        el_ast_dump_expr(node->as.cast.expr, indent + 2, out);
        el_ast_dump_print_indent(indent + 1, out);
        fprintf(out, "type:\n");
        el_ast_dump_type(node->as.cast.type, indent + 2, out);
        break;
    case EL_AST_EXPR_MEMBER:
        el_ast_dump_print_indent(indent, out);
        fprintf(out, "MemberExpr(\""EL_SV_FMT"\"):\n", EL_SV_FARG(node->as.member.name->name));
        el_ast_dump_print_indent(indent + 1, out);
        fprintf(out, "expr:\n");
        el_ast_dump_expr(node->as.member.expr, indent + 2, out);
        break;
    case EL_AST_EXPR_TMEMBER:
        el_ast_dump_print_indent(indent, out);
        fprintf(out, "TMemberExpr(%zu):\n", node->as.tmember.index);
        el_ast_dump_print_indent(indent + 1, out);
        fprintf(out, "expr:\n");
        el_ast_dump_expr(node->as.tmember.expr, indent + 2, out);
        break;
    }
}
