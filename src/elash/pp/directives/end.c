#include "../preproc-internals.h"

bool _el_pp_handle_end(ElPreproc* pp, ElSourceSpan dspan) {
    if (pp->block_stack == NULL) {
        return el_diag_report(
            pp->diag, EL_DIAG_ERROR, "pp.stray-end",
            dspan, "#end without matching #if, #func, #while or #for"
        );
    }

    switch (pp->block_stack->kind) {
    case EL_PP_BLOCK_IF:
        _el_pp_pop_if_block(pp);
        return true;

    case EL_PP_BLOCK_FUNC:
    case EL_PP_BLOCK_WHILE:
    case EL_PP_BLOCK_FOR:
        EL_UNREACHABLE("handled elsewhere; should not be reached");
    }

    EL_UNREACHABLE_ENUM_VAL(ElPpBlockKind, pp->block_stack->kind);
}

bool _el_pp_skip_end(ElPreproc* pp) {
    if (pp->skip_depth > 1) {
        pp->skip_depth--;
        return true;
    }

    EL_ASSERT(pp->block_stack != NULL, "#end at skip_depth 1 without a block");
    pp->skip_depth = 0;

    switch (pp->block_stack->kind) {
    case EL_PP_BLOCK_IF:
        _el_pp_pop_if_block(pp);
        return true;

    case EL_PP_BLOCK_FUNC:
        return _el_pp_finish_pending_func(pp);

    case EL_PP_BLOCK_WHILE:
    case EL_PP_BLOCK_FOR:
        return _el_pp_finish_loop(pp);
    }

    EL_UNREACHABLE_ENUM_VAL(ElPpBlockKind, pp->block_stack->kind);
}
