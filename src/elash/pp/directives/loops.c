#include "../preproc-internals.h"

#include <elash/lexer/tokarr.h>
#include <elash/lexer/tokbuf.h>
#include <elash/pp/value.h>
#include <elash/util/dynarena.h>

#include <string.h>

#define ENSURE_INSIDE_LOOP(PP, DIR, DSPAN, LOOP) do {             \
    (LOOP) = (PP)->block_stack;                                   \
    while ((LOOP) != NULL && !is_loop((LOOP)->kind)) {            \
        (LOOP) = (LOOP)->parent;                                  \
    }                                                             \
                                                                  \
    if ((LOOP) == NULL) {                                         \
        return el_diag_report(                                    \
            (PP)->diag, EL_DIAG_ERROR, "pp." DIR "-outside-loop", \
            (DSPAN), "'#" DIR "' can only be used inside loops"   \
        );                                                        \
    }                                                             \
} while (0)

static ElTokenArray clone_tokbuf(ElPreproc* pp, const ElTokenBuf* buf) {
    if (buf->len == 0)
        return EL_TOKARR_NULL;

    ElToken* out = EL_DYNARENA_NEW_ARR(pp->iarena, ElToken, buf->len);
    memcpy(out, buf->data, buf->len * sizeof(ElToken));

    return (ElTokenArray) {
        .data = out, .count = buf->len
    };
}

static bool capture_line_tokens(ElPreproc* pp, ElTokenBuf* buf) {
    ElToken tok;
    while (_el_pp_read(pp, &tok)) {
        el_tkbuf_push(buf, tok);
    }
    return true;
}

static bool is_loop(ElPpBlockKind kind) {
    return kind == EL_PP_BLOCK_WHILE || kind == EL_PP_BLOCK_FOR;
}

static bool eval_while_cond(ElPreproc* pp, ElTokenArray toks, ElSourceSpan dspan, bool* out) {
    EL_ASSERT(toks.count > 0, "while condition must not be empty");

    ElTokenArrayStream ctx;
    ElTokenStream stream = el_tokarr_as_stream(&ctx, toks);
    _el_pp_push_eval_frame(pp, stream);

    ElPpValue* val = _el_pp_eval(pp);
    _el_pp_pop_frame(pp);

    if (val == NULL) return false;

    if (!_el_pp_ensure_bool(pp, val, dspan, EL_SV("while")))
        return false;

    *out = val->as.bool_;
    return true;
}

static bool enter_loop_body(ElPreproc* pp) {
    EL_ASSERT(pp->block_stack != NULL, "no active block");
    EL_ASSERT(is_loop(pp->block_stack->kind), "top block is not a loop");

    ElPpLoopState* loop = &pp->block_stack->as.loop;

    pp->operation_count += EL_PP_ITER_OPS;
    if (!_el_pp_ensure_ops_available(pp, pp->block_stack->open_span))
        return false;

    if (loop->kind == EL_PP_LOOP_FOR) {
        ElPpValue* iterable = loop->as.for_.iterable;
        usize index = loop->as.for_.index++;
        ElPpValue* elem;

        if (iterable->type == EL_PP_TYPE_LIST) {
            elem = iterable->as.list_.values[index];
        } else {
            elem = _el_pp_new_char(pp->iarena, iterable->as.str_.data[index]);
        }

        ElTokenStream stream = el_tokarr_as_stream(&loop->body_stream, loop->body);
        _el_pp_push_while_body_frame(pp, &loop->body_frame, stream);

        ElPpSymbol* sym = _el_pp_new_sym_var(
            pp->iarena, loop->as.for_.name, pp->block_stack->open_span,
            elem, false, false
        );

        el_pp_scope_assign(pp->current_scope, sym->name, sym);
        return true;
    }

    ElTokenStream stream = el_tokarr_as_stream(&loop->body_stream, loop->body);
    _el_pp_push_while_body_frame(pp, &loop->body_frame, stream);
    return true;
}

bool _el_pp_handle_while(ElPreproc* pp, ElSourceSpan dspan) {
    ElTokenBuf cond_buf;
    el_tkbuf_init(&cond_buf);

    if (!capture_line_tokens(pp, &cond_buf)) {
        el_tkbuf_destroy(&cond_buf);
        return false;
    }

    if (cond_buf.len == 0) {
        el_tkbuf_destroy(&cond_buf);
        return el_diag_report(
            pp->diag, EL_DIAG_ERROR, "pp.unexpected-token",
            dspan, "expected condition after #while"
        );
    }

    ElTokenArray cond = clone_tokbuf(pp, &cond_buf);
    el_tkbuf_destroy(&cond_buf);

    bool cond_val = false;
    if (!eval_while_cond(pp, cond, dspan, &cond_val)) {
        return false;
    }

    _el_pp_push_while_block(
        pp, dspan, cond, cond_val
    );

    if (!cond_val) {
        pp->skip_depth++;
        return true;
    }

    el_tkbuf_clear(&pp->capture_buf);
    pp->skip_capture = true;
    pp->skip_depth++;
    return true;
}

bool _el_pp_handle_for(ElPreproc* pp, ElSourceSpan dspan) {
    ElToken name, colon;

    if (!_el_pp_read(pp, &name) || name.type != EL_TT_IDENT) {
        return el_diag_report(
            pp->diag, EL_DIAG_ERROR, "pp.unexpected-token",
            dspan, "expected an identifier after #for"
        );
    }

    if (!_el_pp_read(pp, &colon) || colon.type != EL_TT_COLON) {
        return el_diag_report(
            pp->diag, EL_DIAG_ERROR, "pp.unexpected-token",
            colon.span, "expected ':' after #for variable"
        );
    }

    ElPpValue* iterable = _el_pp_eval(pp);
    if (iterable == NULL) return false;

    if (iterable->type != EL_PP_TYPE_LIST && iterable->type != EL_PP_TYPE_STR) {
        return el_diag_report(
            pp->diag, EL_DIAG_ERROR, "pp.not-iterable",
            colon.span, "value after #for must be a list or string"
        );
    }

    usize count = iterable->type == EL_PP_TYPE_LIST
        ? iterable->as.list_.count
        : iterable->as.str_.len;

    bool has_items = count != 0;
    _el_pp_push_for_block(pp, dspan, name.lexeme, iterable, has_items);

    if (!has_items) {
        pp->skip_depth++;
        return true;
    }

    el_tkbuf_clear(&pp->capture_buf);
    pp->skip_capture = true;
    pp->skip_depth++;
    return true;
}

bool _el_pp_skip_while(ElPreproc* pp) {
    if (!_el_pp_skip_expr(pp)) return false;
    pp->skip_depth++;
    return true;
}

bool _el_pp_skip_for(ElPreproc* pp) {
    ElToken tok;
    if (!_el_pp_read(pp, &tok) || tok.type != EL_TT_IDENT) return false;
    if (!_el_pp_read(pp, &tok) || tok.type != EL_TT_COLON) return false;
    if (!_el_pp_skip_expr(pp)) return false;
    pp->skip_depth++;
    return true;
}

static bool leave_loop_body(ElPreproc* pp, ElPpBlock* loop, bool continues) {
    // this needs to be updated every time new block kinds are added
    while (pp->block_stack != loop) {
        EL_ASSERT(pp->block_stack->kind == EL_PP_BLOCK_IF,
                  "unexpected block kind");
        _el_pp_pop_if_block(pp);
    }

    while (pp->frame->type != FRAME_LOOP_BODY)
        _el_pp_pop_frame(pp);

    _el_pp_pop_frame(pp);
    if (!continues) {
        _el_pp_pop_block(pp);
        return true;
    }

    return _el_pp_loop_body_exhausted(pp);
}

bool _el_pp_handle_continue(ElPreproc* pp, ElSourceSpan dspan) {
    ElPpBlock* loop;
    ENSURE_INSIDE_LOOP(pp, "continue", dspan, loop);
    return leave_loop_body(pp, loop, /*continues=*/true);
}

bool _el_pp_handle_break(ElPreproc* pp, ElSourceSpan dspan) {
    ElPpBlock* loop;
    ENSURE_INSIDE_LOOP(pp, "break", dspan, loop);
    return leave_loop_body(pp, loop, /*continues=*/false);
}

bool _el_pp_skip_continue(ElPreproc* pp) {
    (void)pp;
    return true;
}
bool _el_pp_skip_break(ElPreproc* pp) {
    (void)pp;
    return true;
}

bool _el_pp_finish_loop(ElPreproc* pp) {
    EL_ASSERT(pp->block_stack != NULL, "no active block");
    EL_ASSERT(is_loop(pp->block_stack->kind), "top block is not a loop");

    ElPpLoopState* loop = &pp->block_stack->as.loop;
    if (!loop->capturing_body) {
        _el_pp_pop_block(pp);
        return true;
    }

    loop->body = clone_tokbuf(pp, &pp->capture_buf);

    loop->capturing_body = false;
    pp->skip_capture = false;
    pp->skip_depth = 0;
    el_tkbuf_clear(&pp->capture_buf);

    return enter_loop_body(pp);
}

bool _el_pp_loop_body_exhausted(ElPreproc* pp) {
    EL_ASSERT(pp->block_stack != NULL, "no active block");
    EL_ASSERT(is_loop(pp->block_stack->kind), "top block is not a loop");

    ElPpBlock* block = pp->block_stack;
    ElPpLoopState* loop = &block->as.loop;

    if (loop->kind == EL_PP_LOOP_FOR) {
        ElPpValue* iterable = loop->as.for_.iterable;
        usize count = iterable->type == EL_PP_TYPE_LIST
            ? iterable->as.list_.count
            : iterable->as.str_.len;

        if (loop->as.for_.index >= count) {
            _el_pp_pop_block(pp);
            return true;
        }
        return enter_loop_body(pp);
    }

    bool cond_val = false;
    if (!eval_while_cond(pp, loop->as.while_.cond, block->open_span, &cond_val)) {
        return false;
    }

    if (!cond_val) {
        _el_pp_pop_block(pp);
        return true;
    }
    return enter_loop_body(pp);
}
