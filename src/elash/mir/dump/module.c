#include <elash/mir/dump/module.h>
#include <elash/mir/dump/func.h>

void el_mir_dump_module(const ElMirModule* mod, FILE* out) {
    if (!mod) return;

    for (ElMirFunc* func = mod->first_func; func; func = func->next) {
        el_mir_dump_func(func, 0, out);
        if (func->next) fputs("\n", out);
    }
}
