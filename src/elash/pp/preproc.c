#include "preproc-internals.h"

#include <elash/util/dynarena.h>

bool el_pp_init(
    ElPreproc* pp, ElTokenStream input, const ElSourceDocument* root_doc,
    ElDynArena* arena, const ElPpIncMap* imap
) {
    pp->frame = NULL;
    pp->include_depth = 0;
    pp->skip_depth = 0;
    pp->if_stack = NULL;
    pp->has_lookahead = false;

    pp->imap   = imap;
    pp->farena = arena;

    if (!el_tkque_init(&pp->pending))
        return false;

    pp->iarena = EL_DYNARENA_NEW(pp->farena, ElDynArena);
    if (!el_dynarena_init(pp->iarena))
        return false;

    _el_pp_push_frame(pp, input, root_doc);

    pp->current_scope = NULL;
    pp->builtin_scope = _el_pp_push_scope(pp);
    pp->global_scope  = _el_pp_push_scope(pp);

    return true;
}

void el_pp_free(ElPreproc* pp) {
    EL_ASSERT(pp->current_scope == pp->global_scope, "nested scopes were not popped before destruction");
    el_pp_scope_free(pp->global_scope);
    el_pp_scope_free(pp->builtin_scope);

    el_dynarena_free(pp->iarena);
    el_tkque_destroy(&pp->pending);
}

////////// scopes ////////////
ElPpScope* _el_pp_push_scope(ElPreproc* pp) {
    ElPpScope* scope = el_pp_scope_new(pp->current_scope);
    pp->current_scope = scope;
    return scope;
}

ElPpScope* _el_pp_pop_scope(ElPreproc* pp) {
    ElPpScope* scope = pp->current_scope;
    for (usize i = 0; i < scope->capacity; ++i) {
        if (scope->entries[i].state != _EL_PP_OCCUPIED) continue;
        if (scope->entries[i].value->kind != EL_PP_SYM_VAR) continue;

        ElPpVarSym* var = &scope->entries[i].value->as.var;
        if (!var->is_mutable || var->was_mutated) continue;

        el_diag_report(
            pp->diag, EL_DIAG_WARN, "pp.never-mutated",
            scope->entries[i].value->defspan,
            "variable defined as mutable but never mutated",
        );
        el_diag_help(
            pp->diag, "use #const if you don't need mutability"
        );
    }

    ElPpScope* parent = scope->parent;
    el_pp_scope_free(scope);
    return pp->current_scope = parent;
}

///////// include frames ///////////
void _el_pp_push_frame(ElPreproc* pp, ElTokenStream stream, const ElSourceDocument* doc) {
    pp->include_depth++;
    pp->frame = EL_DYNARENA_NEW_STRUCT(pp->iarena, ElPpFrame, {
        .stream = stream,
        .doc    = doc,
        .parent = pp->frame,
    });

    _el_pp_push_scope(pp);
}

static void el_pp_pop_frame(ElPreproc* pp) {
    pp->frame = pp->frame->parent;
    pp->include_depth--;
    _el_pp_pop_scope(pp);
}

static void frame_unread(ElPreproc* pp, ElToken tok) {
    pp->frame->pushback = tok;
    pp->frame->has_pushback = true;
}

static bool read_from_active_frame(ElPreproc* pp, ElToken* out_tok) {
    if (pp->frame == NULL) {
        return false;
    }

    if (pp->frame->has_pushback) {
        *out_tok = pp->frame->pushback;
        pp->frame->has_pushback = false;
        return true;
    }

    *out_tok = pp->frame->stream.next(&pp->frame->stream, pp->diag);
    return out_tok->type != EL_TT_EOF;
}

////////////// if frames //////////////
void _el_pp_push_if_frame(ElPreproc* pp, bool take_branch, ElSourceSpan ifspan) {
    pp->if_stack = EL_DYNARENA_NEW_STRUCT(pp->iarena, ElPpIfFrame, {
        .branch_taken = take_branch,
        .has_scope    = take_branch,
        .had_else     = false,
        .ifspan       = ifspan,
        .parent       = pp->if_stack,
    });
    if (take_branch) {
        _el_pp_push_scope(pp);
    }
}

void _el_pp_enter_if_branch(ElPreproc* pp) {
    EL_ASSERT(pp->if_stack != NULL, "enter_if_branch with empty if stack");
    EL_ASSERT(!pp->if_stack->has_scope, "enter_if_branch while a branch scope is already open");
    pp->if_stack->branch_taken = true;
    pp->if_stack->has_scope = true;
    _el_pp_push_scope(pp);
}

void _el_pp_leave_if_branch(ElPreproc* pp) {
    EL_ASSERT(pp->if_stack != NULL, "leave_if_branch with empty if stack");
    EL_ASSERT(pp->if_stack->has_scope, "leave_if_branch without an open branch scope");
    pp->if_stack->has_scope = false;
    _el_pp_pop_scope(pp);
}

void _el_pp_pop_if_frame(ElPreproc* pp) {
    EL_ASSERT(pp->if_stack != NULL, "pop_if_frame with empty if stack");
    if (pp->if_stack->has_scope) {
        _el_pp_pop_scope(pp);
    }
    pp->if_stack = pp->if_stack->parent;
}

bool _el_pp_read(ElPreproc* pp, ElToken* out_tok) {
    while (read_from_active_frame(pp, out_tok)) {
        switch (out_tok->type) {
        case EL_TT_WHITESPACE:
        case EL_TT_LINE_COMMENT:
        case EL_TT_BLOCK_COMMENT:
            continue;

        case EL_TT_NEWLINE:
        //case EL_TT_HASH:
            frame_unread(pp, *out_tok);
            return false;

        default:
            return true;
        }
    }

    return false;
}

bool _el_pp_peek(ElPreproc* pp, ElToken* out_tok) {
    if (!_el_pp_read(pp, out_tok)) {
        return false;
    }
    frame_unread(pp, *out_tok);
    return true;
}

bool _el_pp_next_internal(ElPreproc* pp, ElToken* out_tok, bool handle_directives) {
    while (true) {
        ElToken input_tok;

        if (pp->pending.len != 0) {
            el_tkque_pop(&pp->pending, &input_tok);
        } else if (pp->has_lookahead) {
            input_tok = pp->lookahead;
            pp->has_lookahead = false;
        } else if (pp->frame == NULL) {
            if (pp->if_stack != NULL || pp->skip_depth != 0) {
                el_diag_report(
                    pp->diag, EL_DIAG_ERROR, "pp.unterm-if",
                    pp->if_stack->ifspan, "unterminated #if directive"
                );
                pp->if_stack = NULL;
                pp->skip_depth = 0;
            }
            return false;
        } else if (!read_from_active_frame(pp, &input_tok)) {
            el_pp_pop_frame(pp);
            continue;
        }

        switch (input_tok.type) {
        case EL_TT_NEWLINE:
        case EL_TT_WHITESPACE:
        case EL_TT_LINE_COMMENT:
        case EL_TT_BLOCK_COMMENT:
            // skip whitespace, new lines and comments
            continue;

        case EL_TT_HASH:
            if (pp->skip_depth > 0) {
                // my first idea was to just push all tokens from the taken branch
                // to the queue but that would be pretty slow and suboptimal; this
                // approach is better, just skipping tokens in place without spamming
                // the queue (it will be reserved for macro expansion)
                if (!_el_pp_skip_directive(pp, input_tok)) {
                    return false;
                }
                continue;
            }
            if (handle_directives) {
                return _el_pp_preprocess_directive(pp, input_tok, out_tok);
            }
            *out_tok = input_tok;
            return true;

        default:
            if (pp->skip_depth > 0) {
                continue;
            }
            *out_tok = input_tok;
            return true;
        }
    }
}

bool _el_pp_next(ElPreproc* pp, ElToken* out_tok) {
    return _el_pp_next_internal(pp, out_tok, false);
}
bool _el_pp_next_d(ElPreproc* pp, ElToken* out_tok) {
    return _el_pp_next_internal(pp, out_tok, true);
}

bool el_pp_next(ElPreproc* pp, ElToken* out_tok, ElDiagEngine* diag) {
    pp->diag = diag;
    return _el_pp_next_internal(pp, out_tok, true);
}

ElToken _el_pp_advance(ElPreproc* pp) {
    ElToken tok;
    if (!_el_pp_read(pp, &tok)) {
        return (ElToken) { .type = EL_TT_EOF };
    }
    return tok;
}

bool _el_pp_match(ElPreproc* pp, ElTokenType type) {
    ElToken tok;
    if (!_el_pp_peek(pp, &tok) || tok.type != type) {
        return false;

    }
    _el_pp_advance(pp);
    return true;
}

bool _el_pp_expect(ElPreproc* pp, ElTokenType type) {
    ElToken tok = _el_pp_advance(pp);
    if (tok.type == type) {
        return true;
    }

    return el_diag_report(
        pp->diag, EL_DIAG_ERROR, "pp.unexpected-token",
        tok.span, "expected ${expected}, found ${found}",
        EL_DIAG_STRING("expected", el_token_type_format(type)),
        EL_DIAG_TOKEN("found", tok),
    );
}

static ElToken _el_pp_token_stream_next(ElTokenStream* self, ElDiagEngine* diag) {
    ElPreproc* pp = self->ctx;
    ElToken tok;
    if (!el_pp_next(pp, &tok, diag)) {
        return (ElToken){ .type = EL_TT_EOF };
    }
    return tok;
}

ElTokenStream el_pp_as_token_stream(ElPreproc* pp) {
    return (ElTokenStream) {
        .next = _el_pp_token_stream_next,
        .ctx = pp,
    };
}
