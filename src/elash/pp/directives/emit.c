#include "../preproc-internals.h"

#include <elash/pp/value.h>

#include <elash/util/dynarena.h>
#include <elash/util/strbuf.h>
#include <elash/util/todo.h>

#include <inttypes.h>

static bool toktok(ElPreproc* pp, ElTokenType type, ElStringView lexeme, ElSourceSpan span) {
    return el_tkque_push(&pp->pending, (ElToken) {
        .type = type,
        .lexeme = el_dynarena_clone_sv(pp->farena, lexeme),
        .span = span,
    });
}

#define BUFSIZE 32
static bool emit_value(ElPreproc* pp, ElPpValue* value, ElSourceSpan span) {
    char buf[BUFSIZE];
    int len;

    switch (value->type) {
    case EL_PP_TYPE_TOK:
        return el_tkque_push(&pp->pending, value->as.tok_);
    case EL_PP_TYPE_NULL:
        return toktok(pp, EL_TT_NULL_LITERAL, EL_SV("null"), span);
    case EL_PP_TYPE_BOOL:
        return toktok(
            pp,
            value->as.bool_ ? EL_TT_TRUE_LITERAL : EL_TT_FALSE_LITERAL,
            value->as.bool_ ? EL_SV("true") : EL_SV("false"),
            span
        );

    case EL_PP_TYPE_INT:
        len = snprintf(buf, sizeof buf, "%"PRId64, value->as.int_);
        return toktok(pp, EL_TT_INT_LITERAL, el_sv_from_data_and_len(buf, len), span);
    case EL_PP_TYPE_FLOAT:
        len = snprintf(buf, sizeof buf, "%g", value->as.float_);
        return toktok(pp, EL_TT_FLOAT_LITERAL, el_sv_from_data_and_len(buf, len), span);

    case EL_PP_TYPE_LIST: {
        ElPpList list = value->as.list_;
        for (usize i = 0; i < list.count; ++i) {
            if (!emit_value(pp, list.values[i], span)) {
                return false;
            }
        }
        return true;
    }

    case EL_PP_TYPE_CHAR:
        EL_TODO("escaping chars literals is tricky");

    case EL_PP_TYPE_STR:
        return toktok(pp, EL_TT_STRING_LITERAL, value->as.str_, span);
    }

    EL_UNREACHABLE_ENUM_VAL(ElPpType, value->type);
}

static bool handle_splice(ElPreproc* pp, ElToken hash) {
    if (!_el_pp_expect(pp, EL_TT_LBRACE)) {
        return false;
    }

    ElPpValue* value = _el_pp_eval(pp);
    if (value == NULL) {
        return false;
    }

    ElToken t; _el_pp_peek(pp, &t);
    if (!_el_pp_expect(pp, EL_TT_RBRACE)) {
        return false;
    }

    ElSourceSpan espan = el_srcspan_merge(hash.span, t.span);
    return emit_value(pp, value, espan);
}

bool _el_pp_handle_emit(ElPreproc* pp, ElSourceSpan dspan) {
    (void) dspan;

    ElToken tok;
    while (_el_pp_peek(pp, &tok)) {
        if (tok.type == EL_TT_NEWLINE)
            break;

        _el_pp_advance(pp);

        if (tok.type == EL_TT_HASH) {
            ElToken next;
            if (_el_pp_peek(pp, &next) && next.type == EL_TT_LBRACE) {
                if (!handle_splice(pp, tok))
                    return false;
                continue;
            }
        }

        if (!el_tkque_push(&pp->pending, tok)) {
            return false;
        }
    }

    return true;
}

bool _el_pp_skip_emit(ElPreproc* pp) {
    ElToken tok;
    while (_el_pp_peek(pp, &tok)) {
        if (tok.type == EL_TT_NEWLINE)
            break;

        _el_pp_advance(pp);
        if (tok.type != EL_TT_HASH)
            continue;

        ElToken next;
        if (!_el_pp_peek(pp, &next) || next.type != EL_TT_LBRACE)
            continue;

        _el_pp_advance(pp); // {
        if (!_el_pp_skip_expr(pp)) {
            return false;
        }
        if (!_el_pp_match(pp, EL_TT_RBRACE)) {
            return false;
        }
    }
    return true;
}
