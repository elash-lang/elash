#include <elash/ast/dump/unr.h>
#include <elash/ast/dump/indent.h>
#include <elash/ast/dump/expr.h>
#include <elash/ast/tree/unr.h>
#include <elash/defs/sv.h>
#include <elash/util/assert.h>

void el_ast_dump_unr(ElAstUnr* node, usize indent, ElWriter out) {
    el_ast_dump_print_indent(indent, out);
    switch (node->kind) {
    case EL_AST_UNR_IDENT:
        el_writef(
            out, "Unresolved(ident \""EL_SV_FMT"\")\n",
            EL_SV_FARG(node->as.ident->name)
        );
        return;
    case EL_AST_UNR_INDEX:
        el_swrite(out, "Unresolved(index):\n");
        el_ast_dump_print_indent(indent + 1, out);
        el_swrite(out, "base:\n");
        el_ast_dump_unr(node->as.index.base, indent + 2, out);
        el_ast_dump_print_indent(indent + 1, out);
        if (node->as.index.index != NULL) {
            el_swrite(out, "index:\n");
            el_ast_dump_unr(node->as.index.index, indent + 2, out);
        } else {
            el_swrite(out, "index_expr:\n");
            el_ast_dump_expr(node->as.index.index_expr, indent + 2, out);
        }
        return;
    }
    EL_UNREACHABLE_ENUM_VAL(ElAstUnrKind, node->kind);
}
