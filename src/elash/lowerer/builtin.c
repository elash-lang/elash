#include <elash/lowerer/builtin.h>
#include <elash/mir/type.h>

void el_lowerer_init_builtins(ElLowererBuiltins* builtins, ElDynArena* arena) {
    // TODO: replace placeholder width with actual value based on target
    builtins->type_usize = el_mir_new_int_type(arena, 64, true);
    builtins->type_void = el_mir_new_void_type(arena);
}
