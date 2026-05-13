#include <elash/ast/dump/indent.h>

void el_ast_dump_print_indent(usize indent, FILE* out) {
    for (usize i = 0; i < indent; ++i) {
        fputs("  ", out);
    }
}

