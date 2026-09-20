#include "../preproc-internals.h"

#include <elash/lexer/tokarr.h>
#include <elash/lexer/tokbuf.h>
#include <elash/pp/value.h>
#include <elash/util/dynarena.h>

#include <string.h>

// this is temporary. it is currently easily bypassable by just nesting loops.
// this mechanism is basically useless and will be replaced by something like
// a global operation counter that is incremented by every executed directive
// or evaluated expression, no matter if it happens inside a loop, a recursive
// function, recursive include or... all of them at once
#define WHILE_ITER_LIMIT 100000

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
        if (!el_tkbuf_push(buf, tok)) return false;
    }
    return true;
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

static bool while_enter_body(ElPreproc* pp) {
    EL_ASSERT(pp->block_stack != NULL, "no active block");
    EL_ASSERT(pp->block_stack->kind == EL_PP_BLOCK_WHILE, "top block is not #while");

    ElPpWhileState* w = &pp->block_stack->as.while_;
    if (w->iter_count >= WHILE_ITER_LIMIT) {
        return el_diag_report(
            pp->diag, EL_DIAG_ERROR, "pp.while-limit",
            pp->block_stack->open_span,
            "#while iteration limit exceeded"
        );
    }
    w->iter_count++;

    ElTokenStream stream = el_tokarr_as_stream(&w->body_stream, w->body);
    _el_pp_push_while_body_frame(pp, &w->body_frame, stream);
    return true;
}

bool _el_pp_handle_while(ElPreproc* pp, ElSourceSpan dspan) {
    ElTokenBuf cond_buf;
    if (!el_tkbuf_init(&cond_buf)) return false;

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

bool _el_pp_skip_while(ElPreproc* pp) {
    if (!_el_pp_skip_expr(pp)) return false;
    pp->skip_depth++;
    return true;
}

bool _el_pp_finish_while(ElPreproc* pp) {
    EL_ASSERT(pp->block_stack != NULL, "no active block");
    EL_ASSERT(pp->block_stack->kind == EL_PP_BLOCK_WHILE, "top block is not #while");

    ElPpWhileState* w = &pp->block_stack->as.while_;
    if (!w->capturing_body) {
        _el_pp_pop_block(pp);
        return true;
    }

    w->body = clone_tokbuf(pp, &pp->capture_buf);

    w->capturing_body = false;
    pp->skip_capture = false;
    pp->skip_depth = 0;
    el_tkbuf_clear(&pp->capture_buf);

    return while_enter_body(pp);
}

bool _el_pp_while_body_exhausted(ElPreproc* pp) {
    EL_ASSERT(pp->block_stack != NULL, "no active block");
    EL_ASSERT(pp->block_stack->kind == EL_PP_BLOCK_WHILE, "top block is not #while");

    ElPpBlock* block = pp->block_stack;
    ElPpWhileState* w = &block->as.while_;

    bool cond_val = false;
    if (!eval_while_cond(pp, w->cond, block->open_span, &cond_val)) {
        return false;
    }

    if (!cond_val) {
        _el_pp_pop_block(pp);
        return true;
    }
    return while_enter_body(pp);
}
