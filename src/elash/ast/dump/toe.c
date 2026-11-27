#include <elash/ast/dump/toe.h>
#include <elash/ast/dump/unr.h>
#include <elash/ast/dump/indent.h>
#include <elash/ast/dump/type.h>
#include <elash/ast/dump/expr.h>
#include <elash/ast/tree/toe.h>
#include <elash/util/assert.h>

void el_ast_dump_type_or_expr(ElAstToE* node, usize indent, ElWriter* out) {
    el_ast_dump_print_indent(indent, out);
    switch (node->kind) {
    case EL_AST_TOE_TYPE:
        el_swrite(out, "TypeOrExpr(type):\n");
        el_ast_dump_type(node->as.type, indent + 1, out);
        return;
    case EL_AST_TOE_EXPR:
        el_swrite(out, "TypeOrExpr(expr):\n");
        el_ast_dump_expr(node->as.expr, indent + 1, out);
        return;
    case EL_AST_TOE_UNR:
        el_swrite(out, "TypeOrExpr(unresolved):\n");
        el_ast_dump_unr(node->as.unr, indent + 1, out);
        return;
    }
    EL_UNREACHABLE_ENUM_VAL(ElAstToEKind, node->kind);
}
