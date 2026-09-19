#include <elash/hir/symbol/dump.h>
#include <elash/hir/symbol.h>

void el_dump_symbol(ElHirSymbol* symbol, FILE* out) {
    fprintf(out, EL_SV_FMT"#%u", EL_SV_FARG(symbol->name), symbol->id);
}
