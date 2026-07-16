#include <elash/mir/func.h>
#include <elash/mir/value/arg.h>
#include <elash/util/dynarena.h>

#include <stddef.h>

ElMirFunc* el_mir_new_func(ElDynArena* arena, ElMirSymbol* symbol) {
    ElMirFunc* func = EL_DYNARENA_NEW(arena, ElMirFunc);
    *func = (ElMirFunc) {
        .symbol = symbol,
        .first_block = NULL,
        .last_block = NULL,
        .reg_count = 0,
        .block_count = 0,
    };

    if (symbol->kind == EL_MIR_SYM_FUNC) {
        func->arg_count = symbol->as.func.param_count;
        if (func->arg_count > 0) {
            func->args = EL_DYNARENA_NEW_ARR(arena, ElMirValue*, func->arg_count);
            for (uint32_t i = 0; i < func->arg_count; i++) {
                ElMirSymbol* param_sym = symbol->as.func.params[i];
                ElMirType* type = param_sym->as.var.type;
                func->args[i] = el_mir_new_arg(arena, type, i);
            }
        } else {
            func->args = NULL;
        }
    } else {
        func->arg_count = 0;
        func->args = NULL;
    }

    return func;
}

void el_mir_func_append_block(ElMirFunc* func, ElMirBlock* block) {
    if (func->last_block != NULL) {
        func->last_block->next = block;
    } else {
        func->first_block = block;
    }

    func->last_block = block;
    block->next = NULL;
}
