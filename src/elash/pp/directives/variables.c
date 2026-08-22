#include "../preproc-internals.h"

static ElPpSymbol* get_var_symbol(
    ElPreproc* pp, ElStringView dname, ElSourceSpan dspan, ElToken* out_name_tok, bool mut
) {
    if (!_el_pp_read(pp, out_name_tok)) {
        return el_diag_report(
            pp->diag, EL_DIAG_ERROR, "pp.unexpected-token",
            dspan, "expected identifier after #${dir} directive",
            EL_DIAG_STRING("dir", dname)
        );
    }

    if (out_name_tok->type != EL_TT_IDENT) {
         return el_diag_report(
            pp->diag, EL_DIAG_ERROR, "pp.unexpected-token",
            out_name_tok->span, "expected identifier after #${dir} directive",
            EL_DIAG_STRING("dir", dname)
        );
    }

    ElPpSymbol* sym = el_pp_scope_lookup(pp->current_scope, out_name_tok->lexeme);
    if (sym == NULL) {
        return el_diag_report(
            pp->diag, EL_DIAG_ERROR, "pp.undeclared",
            out_name_tok->span, "undeclared identifier ${name} in #${dir} directive",
            EL_DIAG_STRING("name", out_name_tok->lexeme),
            EL_DIAG_STRING("dir", dname)
        );

        return sym;
    }

    if (sym->kind != EL_PP_SYM_VAR) {
        return el_diag_report(
            pp->diag, EL_DIAG_ERROR, "pp.sym-kind",
            out_name_tok->span, "expected a variable name"
        );
    }

    if (mut && !sym->as.var.is_mutable) {
        return el_diag_report(
            pp->diag, EL_DIAG_ERROR, "pp.immutable",
            out_name_tok->span, "constant ${name} cannot be mutated",
            EL_DIAG_STRING("name", sym->name),
        );
    }

    if (mut)
        sym->as.var.was_mutated = true;
    return sym;
}

static bool handle_var_slash_const(ElPreproc* pp, ElSourceSpan dspan, bool mut) {
    ElToken name_tok;
    if (!_el_pp_read(pp, &name_tok) || name_tok.type != EL_TT_IDENT) {
        return el_diag_report(
            pp->diag, EL_DIAG_ERROR, "pp.unexpected-token",
            dspan, "expected identifier after #${dir} directive",
            EL_DIAG_STRING("dir", mut ? EL_SV("var") : EL_SV("const")),
        );
    }

    ElPpSymbol* sym = el_pp_scope_lookup_local(pp->current_scope, name_tok.lexeme);
    if (sym != NULL) {
        if (sym->kind != EL_PP_SYM_VAR) {
            return el_diag_report(
                pp->diag, EL_DIAG_ERROR, "pp.redefinition",
                name_tok.span, "redefinition of ${name} as a different kind of symbol",
                EL_DIAG_STRING("name", name_tok.lexeme)
            );
        }

        return el_diag_report(
            pp->diag, EL_DIAG_ERROR, "pp.redefinition",
            name_tok.span, "redefinition of ${kind} ${name}",
            EL_DIAG_STRING("kind", sym->as.var.is_mutable ? EL_SV("variable") : EL_SV("constant")),
            EL_DIAG_STRING("name", name_tok.lexeme)
        );
    }

    ElToken next; ElPpValue* value;
    if (_el_pp_peek(pp, &next) && next.type == EL_TT_ASSIGN) {
        _el_pp_advance(pp); // '='

        value = _el_pp_eval(pp);
        if (value == NULL) return false;

    } else {
        value = _el_pp_new_null(pp->iarena);
    }

    ElSourceSpan defspan = el_srcspan_merge(dspan, name_tok.span);
    sym = _el_pp_new_sym_var(pp->iarena, name_tok.lexeme, defspan, value, mut);
    return el_pp_scope_assign(pp->current_scope, sym->name, sym);
}

static bool skip_var_slash_const(ElPreproc* pp) {
    ElToken name_tok;
    if (!_el_pp_read(pp, &name_tok) || name_tok.type != EL_TT_IDENT) return false;
    ElToken next;
    if (_el_pp_peek(pp, &next) && next.type == EL_TT_ASSIGN) {
        _el_pp_advance(pp);
        return _el_pp_skip_expr(pp);
    }
    return true;
}

bool _el_pp_handle_var(ElPreproc* pp, ElSourceSpan dspan) {
    return handle_var_slash_const(pp, dspan, true);
}
bool _el_pp_handle_const(ElPreproc* pp, ElSourceSpan dspan) {
    return handle_var_slash_const(pp, dspan, false);
}

bool _el_pp_skip_var(ElPreproc* pp) {
    return skip_var_slash_const(pp);
}
bool _el_pp_skip_const(ElPreproc* pp) {
    return skip_var_slash_const(pp);
}

bool _el_pp_handle_set(ElPreproc* pp, ElSourceSpan dspan) {
    ElToken name_tok;
    ElPpSymbol* sym = get_var_symbol(pp, EL_SV("set"), dspan, &name_tok, true);
    if (sym == NULL) return false;

    if (!_el_pp_match(pp, EL_TT_ASSIGN)) {
        ElToken tok; _el_pp_peek(pp, &tok);
        return el_diag_report(
            pp->diag, EL_DIAG_ERROR, "pp.unexpected-token",
            tok.span, "expected '=' after variable name"
        );
    }

    ElPpValue* value = _el_pp_eval(pp);
    if (value == NULL) return false;

    sym->as.var.v = value;
    return true;
}

bool _el_pp_skip_set(ElPreproc* pp) {
    ElToken name_tok;
    if (!_el_pp_read(pp, &name_tok) || name_tok.type != EL_TT_IDENT) return false;
    if (!_el_pp_match(pp, EL_TT_ASSIGN)) return false;
    return _el_pp_skip_expr(pp);
}

static bool _el_pp_handle_incdec(ElPreproc* pp, ElStringView dname, bool increment, ElSourceSpan dspan) {
    ElToken name_tok;
    ElPpSymbol* sym = get_var_symbol(pp, dname, dspan, &name_tok, true);
    if (sym == NULL) return false;

    ElPpValue* val = sym->as.var.v;
    if (val->type == EL_PP_TYPE_INT) {
        if (increment) val->as.int_++;   else val->as.int_--;
    } else if (val->type == EL_PP_TYPE_FLOAT) {
        if (increment) val->as.float_++; else val->as.float_--;
    } else if (val->type == EL_PP_TYPE_CHAR) {
        if (increment) val->as.char_++;  else val->as.char_--;
    } else {
        return el_diag_report(
            pp->diag, EL_DIAG_ERROR, "pp.invalid-type",
            name_tok.span, "cannot ${op} variable of type ${type}",
            EL_DIAG_STRING("op", increment ? EL_SV("increment") : EL_SV("decrement")),
            EL_DIAG_STRING("type", _el_pp_type_name(val->type))
        );
    }
    return true;
}

bool _el_pp_handle_inc(ElPreproc* pp, ElSourceSpan dspan) {
    return _el_pp_handle_incdec(pp, EL_SV("inc"), true, dspan);
}

bool _el_pp_handle_dec(ElPreproc* pp, ElSourceSpan dspan) {
    return _el_pp_handle_incdec(pp, EL_SV("dec"), false, dspan);
}

bool _el_pp_skip_incdec(ElPreproc* pp) {
    ElToken name_tok;
    return _el_pp_read(pp, &name_tok) && name_tok.type == EL_TT_IDENT;
}
