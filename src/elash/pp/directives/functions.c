#include "../preproc-internals.h"

#include <elash/util/dynarena.h>
#include <elash/lexer/tokbuf.h>

#include <string.h>

static bool check_redefinition(ElPreproc* pp, ElToken name_tok) {
    ElPpSymbol* sym = el_pp_scope_lookup_local(pp->current_scope, name_tok.lexeme);
    if (sym == NULL) return true;

    return el_diag_report(
        pp->diag, EL_DIAG_ERROR, "pp.redefinition",
        name_tok.span, "redefinition of ${kind} ${name}",
        EL_DIAG_STRING("kind", _el_pp_sym_kind_to_string(sym)),
        EL_DIAG_STRING("name", name_tok.lexeme)
    );
}

static bool parse_func_signature(
    ElPreproc* pp, ElSourceSpan dspan, bool* out_public,
    ElToken* out_name, ElPpParamList* out_params
) {
    *out_public = true;

    ElToken tok;
    if (!_el_pp_read(pp, &tok)) {
        return el_diag_report(
            pp->diag, EL_DIAG_ERROR, "pp.unexpected-token",
            dspan, "expected identifier after #func"
        );
    }

    if (tok.type == EL_TT_KW_INTERNAL) {
        *out_public = false;
        if (!_el_pp_read(pp, &tok)) {
            return el_diag_report(
                pp->diag, EL_DIAG_ERROR, "pp.unexpected-token",
                dspan, "expected identifier after #func internal"
            );
        }
    }

    if (tok.type != EL_TT_IDENT) {
        return el_diag_report(
            pp->diag, EL_DIAG_ERROR, "pp.unexpected-token",
            tok.span, "expected identifier after #func"
        );
    }

    *out_name = tok;

    if (!_el_pp_expect(pp, EL_TT_LPAREN)) return false;

    *out_params = (ElPpParamList){0};

    if (!_el_pp_match(pp, EL_TT_RPAREN)) {
        while (true) {
            ElToken param;
            if (!_el_pp_read(pp, &param) || param.type != EL_TT_IDENT) {
                return el_diag_report(
                    pp->diag, EL_DIAG_ERROR, "pp.unexpected-token",
                    dspan, "expected parameter name in #func"
                );
            }

            _el_pp_append_param(out_params, pp->iarena, param.lexeme);

            if (!_el_pp_match(pp, EL_TT_COMMA)) {
                return _el_pp_expect(pp, EL_TT_RPAREN);
            }
            if (_el_pp_match(pp, EL_TT_RPAREN)) break;
        }
    }

    return true;
}

static bool skip_func_signature(ElPreproc* pp) {
    ElToken tok;
    if (!_el_pp_read(pp, &tok)) return false;
    if (tok.type == EL_TT_KW_INTERNAL) {
        if (!_el_pp_read(pp, &tok)) return false;
    }
    if (tok.type != EL_TT_IDENT)         return false;
    if (!_el_pp_match(pp, EL_TT_LPAREN)) return false;

    if (!_el_pp_match(pp, EL_TT_RPAREN)) {
        while (true) {
            ElToken param;
            if (!_el_pp_read(pp, &param) || param.type != EL_TT_IDENT) return false;

            if (!_el_pp_match(pp, EL_TT_COMMA))
                return _el_pp_match(pp, EL_TT_RPAREN);

            if (_el_pp_match(pp, EL_TT_RPAREN)) return true;
        }
    }

    return true;
}

bool _el_pp_finish_pending_func(ElPreproc* pp) {
    EL_ASSERT(pp->block_stack != NULL,                   "no active block");
    EL_ASSERT(pp->block_stack->kind == EL_PP_BLOCK_FUNC, "top block is not #func");

    ElTokenArray body = {
        .count = pp->capture_buf.len,
    };
    if (body.count > 0) {
        body.data = EL_DYNARENA_NEW_ARR(pp->iarena, ElToken, body.count);
        memcpy(body.data, pp->capture_buf.data, body.count * sizeof(ElToken));
    }

    ElPpFuncState* f = &pp->block_stack->as.func;
    ElPpSymbol* sym = _el_pp_new_sym_func(
        pp->iarena,

        f->name,
        pp->block_stack->open_span,
        f->is_public,

        .params   = f->params,
        .body     = body,
    );

    bool ok = el_pp_scope_assign(pp->current_scope, sym->name, sym);

    _el_pp_pop_block(pp);
    pp->skip_capture = false;
    pp->skip_depth = 0;
    el_tkbuf_clear(&pp->capture_buf);
    return ok;
}

bool _el_pp_handle_func(ElPreproc* pp, ElSourceSpan dspan) {
    bool is_public = true;
    ElToken name_tok;

    ElPpParamList params;
    if (!parse_func_signature(pp, dspan, &is_public, &name_tok, &params)) {
        return false;
    }

    if (!check_redefinition(pp, name_tok)) {
        return false;
    }

    ElSourceSpan defspan = el_srcspan_merge(dspan, name_tok.span);
    _el_pp_push_func_block(
        pp, defspan,
        is_public, name_tok.lexeme, params
    );

    el_tkbuf_clear(&pp->capture_buf);
    pp->skip_capture = true;
    pp->skip_depth++;
    return true;
}

bool _el_pp_skip_func(ElPreproc* pp) {
    if (!skip_func_signature(pp)) return false;
    pp->skip_depth++;
    return true;
}

bool _el_pp_handle_return(ElPreproc* pp, ElSourceSpan dspan) {
    if (pp->call_stack == NULL) {
        return el_diag_report(
            pp->diag, EL_DIAG_ERROR, "pp.stray-return",
            dspan, "#return outside of a function"
        );
    }

    ElPpValue* value = _el_pp_eval(pp);
    if (value == NULL) return false;

    pp->call_stack->return_value = value;
    pp->call_stack->has_returned = true;
    return true;
}

bool _el_pp_skip_return(ElPreproc* pp) {
    return _el_pp_skip_expr(pp);
}
