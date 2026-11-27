#include <elash/ast/tree/init.h>
#include <elash/ast/tree/init/desig.h>
#include <elash/ast/dump/indent.h>
#include <elash/ast/dump/init.h>
#include <elash/ast/dump/expr.h>

#include <elash/util/assert.h>

static void el_ast_dump_desig(ElAstDesignator* desig, ElWriter out) {
    switch (desig->kind) {
    case EL_AST_DESIGNATOR_MEMBER:
        el_writef(out, ".%.*s", (int)desig->as.member.len, desig->as.member.data);
        break;
    case EL_AST_DESIGNATOR_TMEMBER:
        el_writef(out, ".%zu", desig->as.tmember);
        break;
    case EL_AST_DESIGNATOR_INDEX:
        el_swrite(out, "[index]");
        break;
    }
}

void el_ast_dump_init(ElAstInit* init, usize indent, ElWriter out) {
    switch (init->kind) {
    case EL_AST_INIT_EXPR:
        return el_ast_dump_expr(init->expr, indent, out);
    case EL_AST_INIT_EMPTY:
        el_ast_dump_print_indent(indent, out);
        el_swrite(out, "EmptyInit\n");
        return;
    case EL_AST_INIT_LIST:
        el_ast_dump_print_indent(indent, out);
        el_writef(out, "InitList(elems=%zu):\n", init->list.count);
        for (ElAstInit* n = init->list.head; n != NULL; n = n->next) {
            el_ast_dump_init(n, indent + 1, out);
        }
        return;
    case EL_AST_INIT_DESIG:
        el_ast_dump_print_indent(indent, out);
        el_writef(out, "DesignatedInit(elems=%zu):\n", init->desig.count);
        for (ElAstDesigInitElem* e = init->desig.head; e != NULL; e = e->next) {
            el_ast_dump_print_indent(indent + 1, out);
            el_swrite(out, "Elem(");
            for (ElAstDesignator* d = e->head; d != NULL; d = d->next) {
                el_ast_dump_desig(d, out);
            }
            el_swrite(out, "):\n");
            el_ast_dump_init(e->init, indent + 2, out);
        }
        return;
    }

    EL_UNREACHABLE_ENUM_VAL(ElAstInitKind, init->kind);
}
