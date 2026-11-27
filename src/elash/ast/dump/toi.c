#include <elash/ast/dump/toi.h>
#include <elash/ast/dump/unr.h>
#include <elash/ast/dump/indent.h>
#include <elash/ast/dump/type.h>
#include <elash/ast/dump/init.h>
#include <elash/ast/tree/toi.h>
#include <elash/util/assert.h>

void el_ast_dump_type_or_init(ElAstToI* node, usize indent, ElWriter* out) {
    el_ast_dump_print_indent(indent, out);
    switch (node->kind) {
    case EL_AST_TOI_TYPE:
        el_swrite(out, "TypeOrInit(type):\n");
        el_ast_dump_type(node->as.type, indent + 1, out);
        return;
    case EL_AST_TOI_INIT:
        el_swrite(out, "TypeOrInit(init):\n");
        el_ast_dump_init(node->as.init, indent + 1, out);
        return;
    case EL_AST_TOI_UNR:
        el_swrite(out, "TypeOrInit(unresolved):\n");
        el_ast_dump_unr(node->as.unr, indent + 1, out);
        return;
    }
    EL_UNREACHABLE_ENUM_VAL(ElAstToIKind, node->kind);
}
