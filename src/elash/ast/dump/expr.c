#include <elash/ast/dump/expr.h>
#include <elash/ast/tree/expr.h>
#include <elash/ast/tree/toi.h>

#include <elash/ast/dump/indent.h>
#include <elash/ast/dump/type.h>
#include <elash/ast/dump/init.h>
#include <elash/ast/dump/toi.h>
#include <inttypes.h>

void el_ast_dump_expr_lit(ElAstLiteral* lit, usize indent, ElWriter* out) {
    el_ast_dump_print_indent(indent, out);
    switch (lit->kind) {
    case EL_AST_LIT_BOOL:
        el_writef(out, "BoolLiteral(%s)\n", lit->of.bool_ ? "true" : "false");
        break;
    case EL_AST_LIT_NULL:
        el_swrite(out, "NullLiteral\n");
        break;
    case EL_AST_LIT_CHAR:
        el_writef(out, "CharLiteral(%c)\n", lit->of.char_);
        break;
    case EL_AST_LIT_INT:
        // NOLINTBEGIN(readability-magic-numbers): You know the rules and so do I
        el_writef(out, "IntLiteral("EL_SV_FMT")\n",
                EL_SV_FARG(el_i128_to_string(lit->of.int_, 10, (char[42]){})));
        // NOLINTEND(readability-magic-numbers)
        break;
    case EL_AST_LIT_FLOAT:
        el_writef(out, "FloatLiteral(%lf)\n", lit->of.float_);
        break;
    case EL_AST_LIT_STRING:
        el_writef(out, "StringLiteral(\""EL_SV_FMT"\")\n", EL_SV_FARG(lit->of.str_));
        break;
    }
}

void el_ast_dump_expr_ident(ElAstIdent* node, usize indent, ElWriter* out) {
    el_ast_dump_print_indent(indent, out);
    el_writef(out, "IdentExpr(\""EL_SV_FMT"\")\n", EL_SV_FARG(node->name));
}

void el_ast_dump_expr(ElAstExpr* node, usize indent, ElWriter* out) {
    switch (node->type) {
    case EL_AST_EXPR_BINARY: {
        ElStringView op = el_bin_op_to_string(node->as.binary.op);
        el_ast_dump_print_indent(indent, out);
        el_writef(out, "BinaryExpr('"EL_SV_FMT"'):\n", EL_SV_FARG(op));
        el_ast_dump_print_indent(indent + 1, out);
        el_swrite(out, "left: \n"); el_ast_dump_expr(node->as.binary.left,  indent + 2, out);
        el_ast_dump_print_indent(indent + 1, out);
        el_swrite(out, "right: \n"); el_ast_dump_expr(node->as.binary.right, indent + 2, out);
        break;
    }
    case EL_AST_EXPR_UNARY: {
        ElStringView op = el_unary_op_format(node->as.unary.op);
        el_ast_dump_print_indent(indent, out);
        el_writef(out, "UnaryExpr('"EL_SV_FMT"'):\n", EL_SV_FARG(op));
        el_ast_dump_expr(node->as.unary.operand, indent + 1, out);
        break;
    }
    case EL_AST_EXPR_LITERAL:
        return el_ast_dump_expr_lit(&node->as.literal, indent, out);
    case EL_AST_EXPR_IDENT:
        return el_ast_dump_expr_ident(&node->as.ident, indent, out);
    case EL_AST_EXPR_CALL: {
        el_ast_dump_print_indent(indent, out);
        el_writef(out, "CallExpr(args=%zu):\n", node->as.call.arg_count);
        el_ast_dump_print_indent(indent + 1, out);
        el_swrite(out, "callee:\n");
        el_ast_dump_expr(node->as.call.callee, indent + 2, out);
        el_ast_dump_print_indent(indent + 1, out);
        el_swrite(out, "args:\n");
        for (ElAstToI* arg = node->as.call.args; arg != NULL; arg = arg->next) {
            el_ast_dump_type_or_init(arg, indent + 2, out);
        }
        break;
    }
    case EL_AST_EXPR_TYPEDINIT:
        el_ast_dump_print_indent(indent, out);
        el_swrite(out, "TypedInit:\n");
        el_ast_dump_print_indent(indent + 1, out);
        el_swrite(out, "type:\n");
        el_ast_dump_type(node->as.typedinit.type, indent + 2, out);
        el_ast_dump_print_indent(indent + 1, out);
        el_swrite(out, "init:\n");
        el_ast_dump_init(node->as.typedinit.init, indent + 2, out);
        break;
    case EL_AST_EXPR_CAST:
        el_ast_dump_print_indent(indent, out);
        el_swrite(out, "CastExpr:\n");
        el_ast_dump_print_indent(indent + 1, out);
        el_swrite(out, "expr:\n");
        el_ast_dump_expr(node->as.cast.expr, indent + 2, out);
        el_ast_dump_print_indent(indent + 1, out);
        el_swrite(out, "type:\n");
        el_ast_dump_type(node->as.cast.type, indent + 2, out);
        break;
    case EL_AST_EXPR_MEMBER:
        el_ast_dump_print_indent(indent, out);
        el_writef(out, "MemberExpr(\""EL_SV_FMT"\"):\n", EL_SV_FARG(node->as.member.name));
        el_ast_dump_print_indent(indent + 1, out);
        el_swrite(out, "expr:\n");
        el_ast_dump_expr(node->as.member.expr, indent + 2, out);
        break;
    case EL_AST_EXPR_TMEMBER:
        el_ast_dump_print_indent(indent, out);
        el_writef(out, "TMemberExpr(%zu):\n", node->as.tmember.index);
        el_ast_dump_print_indent(indent + 1, out);
        el_swrite(out, "expr:\n");
        el_ast_dump_expr(node->as.tmember.expr, indent + 2, out);
        break;
    }
}
