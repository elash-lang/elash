#include <elash/ast/dump/toe.h>
#include <elash/ast/dump/indent.h>
#include <elash/ast/dump/type.h>
#include <elash/ast/dump/expr.h>
#include <elash/ast/tree/toe.h>
#include <elash/defs/sv.h>

#include <elash/util/assert.h>

void el_ast_dump_type_or_expr(ElAstTypeOrExpr* node, usize indent, FILE* out) {
    el_ast_dump_print_indent(indent, out);
    switch (node->kind) {
    case EL_AST_TOE_TYPE:
        fprintf(out, "TypeOrExpr(type):\n");
        el_ast_dump_type(node->as.type, indent + 1, out);
        return;
    case EL_AST_TOE_EXPR:
        fprintf(out, "TypeOrExpr(expr):\n");
        el_ast_dump_expr(node->as.expr, indent + 1, out);
        return;
    case EL_AST_TOE_UNR_IDENT:
        fprintf(
            out, "TypeOrExpr(unresolved ident \""EL_SV_FMT"\")\n",
            EL_SV_FARG(node->as.unr_ident->name)
        );
        return;
    case EL_AST_TOE_UNR_INDEX:
        fprintf(out, "TypeOrExpr(unresolved index):\n");
        el_ast_dump_print_indent(indent + 1, out);
        fprintf(out, "base:\n");
        el_ast_dump_type_or_expr(node->as.unr_index.base, indent + 2, out);
        el_ast_dump_print_indent(indent + 1, out);
        fprintf(out, "index:\n");
        el_ast_dump_type_or_expr(node->as.unr_index.index, indent + 2, out);
        return;
    }
    EL_UNREACHABLE_ENUM_VAL(ElAstTypeOrExprKind, node->kind);
}
