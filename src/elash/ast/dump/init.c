#include <elash/ast/tree/common/init.h>
#include <elash/ast/dump/indent.h>
#include <elash/ast/dump/init.h>
#include <elash/ast/dump/expr.h>

#include <elash/util/assert.h>

void el_ast_dump_init(ElAstInit* init, usize indent, FILE* out) {
    switch (init->kind) {
    case EL_AST_INIT_EXPR:
        return el_ast_dump_expr(init->expr, indent, out);
    case EL_AST_INIT_LIST:
        el_ast_dump_print_indent(indent, out);
        fprintf(out, "InitList(elems=%zu):\n", init->list.count);
        for (ElAstInit* n = init->list.head; n != NULL; n = n->next) {
            el_ast_dump_init(n, indent + 1, out);
        }
        return;
    }

    EL_UNREACHABLE_ENUM_VAL(ElAstInitKind, init->kind);
}
