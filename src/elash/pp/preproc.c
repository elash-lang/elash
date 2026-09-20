#include "preproc-internals.h"

#include <elash/util/dynarena.h>
#include <elash/lexer/tokbuf.h>

bool el_pp_init(
    ElPreproc* pp, ElTokenStream input, const ElSourceDocument* root_doc,
    ElDynArena* arena, const ElPpIncMap* imap, ElProfState* prof
) {
    pp->frame = NULL;
    pp->include_depth = 0;
    pp->skip_depth = 0;
    pp->if_stack = NULL;
    pp->has_lookahead = false;
    pp->skip_capture = false;

    pp->pending_func = (ElPpPendingFunc) {0};
    pp->call_stack = NULL;
    pp->call_depth = 0;

    pp->imap   = imap;
    pp->farena = arena;
    pp->prof  = prof;
    pp->prof_stage = el_prof_current_stage(prof);

    if (pp->prof != NULL) {
        pp->pss_directive = el_prof_new_sub(prof, EL_SV("Processing directives"));
        pp->pss_eval      = el_prof_new_sub(prof, EL_SV("Evaluating expressions"));
    }

    if (!el_tkque_init(&pp->pending))
        return false;
    if (!el_tkbuf_init(&pp->capture_buf))
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
    el_tkbuf_destroy(&pp->capture_buf);
}

////////// scopes ////////////
static void warn_never_mutated_entry(ElPreproc* pp, ElPpSymbol* sym) {
    if (sym->kind != EL_PP_SYM_VAR) return;

    ElPpVarSym* var = &sym->as.var;
    if (!var->is_mutable || var->was_mutated) return;

    el_diag_report(
        pp->diag, EL_DIAG_WARN, "pp.never-mutated",
        sym->defspan,
        "variable defined as mutable but never mutated",
    );
    el_diag_help(
        pp->diag, "use #const if you don't need mutability"
    );
}

static void warn_never_mutated(ElPreproc* pp, ElPpScope* scope) {
    for (usize i = 0; i < scope->capacity; ++i) {
        if (scope->entries[i].state != _EL_PP_OCCUPIED) continue;
        warn_never_mutated_entry(pp, scope->entries[i].value);
    }
}

static void promote_public(ElPreproc* pp, ElPpScope* scope) {
    ElPpScope* parent = scope->parent;
    if (parent == NULL) return;

    for (usize i = 0; i < scope->capacity; ++i) {
        if (scope->entries[i].state != _EL_PP_OCCUPIED) continue;

        ElPpSymbol* sym = scope->entries[i].value;
        if (!sym->is_public) {
            warn_never_mutated_entry(pp, sym);
            continue;
        }

        ElPpSymbol* existing = el_pp_scope_lookup_local(parent, sym->name);
        if (existing != NULL) {
            ElStringView kind = _el_pp_sym_kind_to_string(sym);
            el_diag_report(
                pp->diag, EL_DIAG_ERROR, "pp.redefinition",
                sym->defspan, "redefinition of ${kind} ${name}",
                EL_DIAG_STRING("kind", kind),
                EL_DIAG_STRING("name", sym->name),
            );
            warn_never_mutated_entry(pp, sym);
            continue;
        }

        el_pp_scope_assign(parent, sym->name, sym);
    }
}

ElPpScope* _el_pp_push_scope(ElPreproc* pp) {
    ElPpScope* scope = el_pp_scope_new(pp->current_scope);
    pp->current_scope = scope;
    return scope;
}

ElPpScope* _el_pp_pop_scope(ElPreproc* pp) {
    ElPpScope* scope = pp->current_scope;
    if (scope->promote_on_pop) {
        promote_public(pp, scope);
    } else {
        warn_never_mutated(pp, scope);
    }

    ElPpScope* parent = scope->parent;
    el_pp_scope_free(scope);
    return pp->current_scope = parent;
}

///////// include frames ///////////
void _el_pp_push_frame(ElPreproc* pp, ElTokenStream stream, const ElSourceDocument* doc) {
    ElPpFrame* parent = pp->frame;

    pp->include_depth++;
    pp->frame = EL_DYNARENA_NEW_STRUCT(pp->iarena, ElPpFrame, {
        .stream = stream,
        .doc    = doc,
        .parent = parent,
        .type   = FRAME_CALL,
    });

    if (parent != NULL) {
        _el_pp_push_scope(pp);
    }
}

void _el_pp_push_call_body_frame(ElPreproc* pp, ElTokenStream stream) {
    ElPpFrame* parent = pp->frame;

    pp->frame = EL_DYNARENA_NEW_STRUCT(pp->iarena, ElPpFrame, {
        .stream = stream,
        .doc    = parent != NULL ? parent->doc : NULL,
        .parent = parent,
        .type   = FRAME_CALL,
    });

    ElPpScope* scope = _el_pp_push_scope(pp);
    scope->promote_on_pop = false;
}

void _el_pp_pop_frame(ElPreproc* pp) {
    bool nested = pp->frame->parent != NULL;
    FrameType type = pp->frame->type;

    pp->frame = pp->frame->parent;
    if (type == FRAME_INCLUDE) {
        pp->include_depth--;
    }

    if (nested) {
        _el_pp_pop_scope(pp);
    } else {
        warn_never_mutated(pp, pp->global_scope);
    }
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
    EL_ASSERT(pp->if_stack != NULL,     "if stack should not be empty");
    EL_ASSERT(!pp->if_stack->has_scope, "branch scope is already open");

    pp->if_stack->branch_taken = true;
    pp->if_stack->has_scope = true;
    _el_pp_push_scope(pp);
}

void _el_pp_leave_if_branch(ElPreproc* pp) {
    EL_ASSERT(pp->if_stack != NULL,    "if stack should not be empty");
    EL_ASSERT(pp->if_stack->has_scope, "no open branch scope");

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
            if (pp->skip_capture)
                return el_tkbuf_push(&pp->capture_buf, *out_tok);

            return true;
        }
    }

    return false;
}

bool _el_pp_peek(ElPreproc* pp, ElToken* out_tok) {
    bool cap = pp->skip_capture;
    pp->skip_capture = false;

    bool ok = _el_pp_read(pp, out_tok);
    pp->skip_capture = cap;

    if (!ok) return false;
    frame_unread(pp, *out_tok);
    return true;
}

// NOLINTNEXTLINE
bool _el_pp_next_internal(ElPreproc* pp, ElToken* out_tok, bool handle_directives) {
    // ugly but prevents null pointer dereference
    ElToken dummy_tok;
    if (out_tok == NULL) {
        out_tok = &dummy_tok;
    }

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
            _el_pp_pop_frame(pp);
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
                if (pp->skip_capture) {
                    if (!el_tkbuf_push(&pp->capture_buf, input_tok)) {
                        return false;
                    }
                    continue;
                } else {
                    continue;
                }
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
    ElToken tok = {0}; // this zero initialization is not needed however the compiler
                       // is yelling at me and i guess it's the simplest way to fix it

    if (!_el_pp_peek(pp, &tok) || tok.type != type)
        return false;

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
    ElProfScope* scope = el_prof_enter_stage(pp->prof, pp->prof_stage);

    ElToken tok;
    if (!el_pp_next(pp, &tok, diag)) {
        el_prof_leave_stage(pp->prof, scope);
        return (ElToken) { .type = EL_TT_EOF };
    }
    el_prof_leave_stage(pp->prof, scope);
    return tok;
}

ElTokenStream el_pp_as_token_stream(ElPreproc* pp) {
    return (ElTokenStream) {
        .next = _el_pp_token_stream_next,
        .ctx = pp,
        .prof = pp->prof,
        .prof_stage = pp->prof_stage,
    };
}
