#include <elash/hir/dump/indent.h>

void el_hir_dump_print_indent(usize indent, ElWriter out) {
    for (usize i = 0; i < indent; ++i) {
        el_swrite(out, "  ");
    }
}
