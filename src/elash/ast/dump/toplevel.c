#include <elash/ast/dump/toplevel.h>
#include <elash/ast/tree/toplevel.h>

#include <elash/ast/dump/decl.h>

#include <elash/util/assert.h>
#include <stdio.h>

void el_ast_dump_toplevel(ElAstTopLevel* node, usize indent, FILE* out) {
    switch (node->type) {
    case EL_AST_TOPLVL_DECL:
        el_ast_dump_decl(node->as.decl, indent, out);
        return;
    }
    EL_UNREACHABLE_ENUM_VAL(ElAstTopLevelType, node->type);
}
