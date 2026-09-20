#include "preproc-internals.h"

#include <elash/lexer/tokarr.h>

void _el_pp_append_param(ElPpParamList* list, ElDynArena* arena, ElStringView name) {
    ElPpFuncParam* new = EL_DYNARENA_NEW_STRUCT(arena, ElPpFuncParam, {
        .name = name,
        .next = NULL,
    });

    if (list->tail != NULL) {
        list->tail->next = new;
    } else {
        list->head = new;
    }
    list->tail = new;
    list->count++;
}

void _el_pp_append_arg(ElPpArgList* list, ElPpValue* val) {
    val->next = NULL;

    if (list->tail != NULL) {
        list->tail->next = val;
    } else {
        list->head = val;
    }
    list->tail = val;
    list->count++;
}

static void cleanup_call(ElPreproc* pp, ElPpCallFrame* call) {
    while (pp->if_stack != NULL) {
        _el_pp_pop_if_frame(pp);
    }

    pp->if_stack   = call->saved_if_stack;
    pp->skip_depth = call->saved_skip_depth;

    while (pp->frame != NULL && pp->frame != call->caller_frame) {
        _el_pp_pop_frame(pp);
    }

    pp->call_stack = call->parent;
    pp->call_depth--;
}

static ElPpCallFrame* make_call_frame(ElPreproc* pp, ElPpSymbol* sym, ElSourceSpan cspan) {
    return EL_DYNARENA_NEW_STRUCT(pp->iarena, ElPpCallFrame, {
        .return_value = NULL,
        .has_returned = false,
        .call_span = cspan,
        .func = sym,
        .saved_if_stack = pp->if_stack,
        .saved_skip_depth = pp->skip_depth,
        .body_frame = NULL,
        .caller_frame = pp->frame,
        .parent = pp->call_stack,
    });
}

static ElPpValue* execute_function(
    ElPreproc* pp, ElPpSymbol* sym, ElPpArgList args, ElSourceSpan cspan
) {
    ElPpFuncSym* func = &sym->as.func;

    ElTokenArrayStream* stream_ctx = EL_DYNARENA_NEW(pp->iarena, ElTokenArrayStream);
    ElTokenStream body_stream = el_token_array_as_stream(stream_ctx, func->body, func->body_len);

    ElPpCallFrame* call =
        make_call_frame(pp, sym, cspan);

    pp->call_stack = call;
    pp->call_depth++;
    pp->if_stack = NULL;
    pp->skip_depth = 0;

    _el_pp_push_call_body_frame(pp, body_stream);
    call->body_frame = pp->frame;

    ElPpValue* arg = args.head;
    ElPpFuncParam* param = func->params.head;
    while (arg != NULL) {
        ElPpSymbol* psym = _el_pp_new_sym_var(
            pp->iarena, param->name, cspan,
            arg, /*mut=*/false, /*is_public=*/false
        );

        if (!el_pp_scope_assign(pp->current_scope, param->name, psym)) {
            cleanup_call(pp, call);
            return NULL;
        }

        arg = arg->next, param = param->next;
    }

    while (!call->has_returned) {
        ElToken tok;
        if (!_el_pp_next_d(pp, &tok)) break;
        if (call->has_returned)       break;

        cleanup_call(pp, call);
        return el_diag_report(
            pp->diag, EL_DIAG_ERROR, "pp.func-token",
            tok.span, "unexpected token in function body"
        );
    }

    if (!call->has_returned) {
        ElSourceSpan span = sym->defspan;
        cleanup_call(pp, call);
        return el_diag_report(
            pp->diag, EL_DIAG_ERROR, "pp.func-no-return",
            span, "function '${name}' reached the end without #return",
            EL_DIAG_STRING("name", sym->name)
        );
    }

    ElPpValue* result = call->return_value;
    cleanup_call(pp, call);
    return result;
}

ElPpValue* _el_pp_call_func(ElPreproc* pp, ElPpSymbol* sym, ElSourceSpan cspan) {
    EL_ASSERT(sym->kind == EL_PP_SYM_FUNC, "expected a function symbol");
    ElPpFuncSym* func = &sym->as.func;

    if (pp->call_depth >= CALL_DEPTH_LIMIT) {
        return el_diag_report(
            pp->diag, EL_DIAG_ERROR, "pp.call-depth",
            cspan, "preprocessor function call depth limit exceeded"
        );
    }

    if (!_el_pp_expect(pp, EL_TT_LPAREN)) return NULL;

    ElPpArgList args = {0};
    if (!_el_pp_match(pp, EL_TT_RPAREN)) {
        while (true) {
            ElPpValue* arg = _el_pp_eval(pp);
            if (arg == NULL) return NULL;

            _el_pp_append_arg(&args, arg);

            if (_el_pp_match(pp, EL_TT_RPAREN)) break;
            if (!_el_pp_expect(pp, EL_TT_COMMA)) return NULL;
        }
    }

    if (args.count != func->params.count) {
        return el_diag_report(
            pp->diag, EL_DIAG_ERROR, "pp.arg-count-mismatch",
            cspan, "expected ${expected} arguments, but got ${got}",
            EL_DIAG_INT("expected", func->params.count),
            EL_DIAG_INT("got", args.count),
        );
    }

    return execute_function(pp, sym, args, cspan);
}
