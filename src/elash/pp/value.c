#include <elash/pp/preproc.h>
#include <elash/pp/value.h>

#include <elash/util/assert.h>

ElStringView _el_pp_type_name(ElPpType type) {
    switch (type) {
    case EL_PP_TYPE_NULL:  return EL_SV("null");
    case EL_PP_TYPE_INT:   return EL_SV("int");
    case EL_PP_TYPE_BOOL:  return EL_SV("bool");
    case EL_PP_TYPE_FLOAT: return EL_SV("float");
    case EL_PP_TYPE_CHAR:  return EL_SV("char");
    case EL_PP_TYPE_LIST:  return EL_SV("list");
    case EL_PP_TYPE_STR:   return EL_SV("string");
    case EL_PP_TYPE_TOK:   return EL_SV("token");
    }
    EL_UNREACHABLE_ENUM_VAL(ElPpType, type);
}

ElPpValue* _el_pp_strcat(ElPreproc* pp, ElPpValue* lhs, ElPpValue* rhs) {
    EL_ASSERT(lhs->type == EL_PP_TYPE_STR && rhs->type == EL_PP_TYPE_STR, "invalid operand types");

    usize len = lhs->as.str_.len + rhs->as.str_.len;
    char* buf = EL_DYNARENA_NEW_ARR(pp->iarena, char, len);

    if (len != 0) {
        memcpy(buf, lhs->as.str_.data, lhs->as.str_.len);
        memcpy(buf + lhs->as.str_.len, rhs->as.str_.data, rhs->as.str_.len);
    }
    return _el_pp_new_str(pp->iarena, el_sv_from_data_and_len(buf, len));
}

ElPpValue* _el_pp_listcat(ElPreproc* pp, ElPpValue* lhs, ElPpValue* rhs) {
    EL_ASSERT(lhs->type == EL_PP_TYPE_LIST && rhs->type == EL_PP_TYPE_LIST, "invalid operand types");

    usize count = lhs->as.list_.count + rhs->as.list_.count;
    ElPpValue** values = NULL;

    if (count != 0) {
        values = EL_DYNARENA_NEW_ARR(pp->iarena, ElPpValue*, count);
        memcpy(values, lhs->as.list_.values, lhs->as.list_.count * sizeof(ElPpValue*));
        memcpy(values + lhs->as.list_.count, rhs->as.list_.values, rhs->as.list_.count * sizeof(ElPpValue*));
    }
    return _el_pp_new_list(pp->iarena, (ElPpList) { values, count });
}

// let's support comparing ints and floats for convenience
static bool is_int_float_compar(ElPpType ltype, ElPpType rtype) {
    return (ltype == EL_PP_TYPE_INT || ltype == EL_PP_TYPE_FLOAT)
        && (rtype == EL_PP_TYPE_INT || rtype == EL_PP_TYPE_FLOAT);
}

#define FOOBARBAZ(OP) do {                                                                \
    if (lhs->type == EL_PP_TYPE_INT && rhs->type == EL_PP_TYPE_INT) {                     \
        return lhs->as.int_ OP rhs->as.int_;                                              \
    }                                                                                     \
    if (is_int_float_compar(lhs->type, rhs->type)) {                                      \
        double l = (lhs->type == EL_PP_TYPE_INT) ? (double)lhs->as.int_ : lhs->as.float_; \
        double r = (rhs->type == EL_PP_TYPE_INT) ? (double)rhs->as.int_ : rhs->as.float_; \
        return l OP r;                                                                    \
    }                                                                                     \
} while (0)

bool _el_pp_value_eq(ElPreproc* pp, ElPpValue* lhs, ElPpValue* rhs, ElSourceSpan span) {
    if (lhs->type != rhs->type) {
        FOOBARBAZ(==);

        return el_diag_report(
            pp->diag, EL_DIAG_ERROR, "pp.compar", span,
            "equality comparison between incompatible types: '${left}' and '${right}'",
            EL_DIAG_STRING("left", _el_pp_type_name(lhs->type)),
            EL_DIAG_STRING("right", _el_pp_type_name(rhs->type))
        );
    }

    switch (lhs->type) {
    case EL_PP_TYPE_NULL:
        return true;
    case EL_PP_TYPE_INT:
        return lhs->as.int_ == rhs->as.int_;
    case EL_PP_TYPE_BOOL:
        return lhs->as.bool_ == rhs->as.bool_;
    case EL_PP_TYPE_FLOAT:
        return lhs->as.float_ == rhs->as.float_;
    case EL_PP_TYPE_CHAR:
        return lhs->as.char_ == rhs->as.char_;
    case EL_PP_TYPE_STR:
        return el_sv_eql(lhs->as.str_, rhs->as.str_);
    case EL_PP_TYPE_LIST:
        if (lhs->as.list_.count != rhs->as.list_.count)
            return false;

        for (usize i = 0; i < lhs->as.list_.count; ++i)
            // TODO: passing `span` here is sloppy.
            if (!_el_pp_value_eq(pp, lhs->as.list_.values[i], rhs->as.list_.values[i], span))
                return false;

        return true;
    case EL_PP_TYPE_TOK:
        return lhs->as.tok_.type == rhs->as.tok_.type
            && el_sv_eql(lhs->as.tok_.lexeme, rhs->as.tok_.lexeme);
    }

    EL_UNREACHABLE_ENUM_VAL(ElPpType, lhs->type);
}

bool _el_pp_value_lt(ElPreproc* pp, ElPpValue* lhs, ElPpValue* rhs, ElSourceSpan span) {
    FOOBARBAZ(<);

    if (lhs->type == EL_PP_TYPE_CHAR && rhs->type == EL_PP_TYPE_CHAR) {
        return lhs->as.char_ < rhs->as.char_;
    }

    return el_diag_report(
        pp->diag, EL_DIAG_ERROR, "pp.compar", span,
        "operator '<' is not defined for '${left}' and '${right}'",
        EL_DIAG_STRING("left", _el_pp_type_name(lhs->type)),
        EL_DIAG_STRING("right", _el_pp_type_name(rhs->type))
    );
}

bool _el_pp_value_gt(ElPreproc* pp, ElPpValue* lhs, ElPpValue* rhs, ElSourceSpan span) {
    FOOBARBAZ(>);

    if (lhs->type == EL_PP_TYPE_CHAR && rhs->type == EL_PP_TYPE_CHAR) {
        return lhs->as.char_ > rhs->as.char_;
    }

    return el_diag_report(
        pp->diag, EL_DIAG_ERROR, "pp.compar", span,
        "operator '>' is not defined for '${left}' and '${right}'",
        EL_DIAG_STRING("left", _el_pp_type_name(lhs->type)),
        EL_DIAG_STRING("right", _el_pp_type_name(rhs->type))
    );
}

//////// clone ////////
ElPpValue* _el_pp_value_clone(ElDynArena* arena, ElPpValue* val) {
    if (val == NULL) return NULL;

    switch (val->type) {
    case EL_PP_TYPE_NULL:  return _el_pp_new_null(arena);
    case EL_PP_TYPE_INT:   return _el_pp_new_int(arena, val->as.int_);
    case EL_PP_TYPE_BOOL:  return _el_pp_new_bool(arena, val->as.bool_);
    case EL_PP_TYPE_FLOAT: return _el_pp_new_float(arena, val->as.float_);
    case EL_PP_TYPE_CHAR:  return _el_pp_new_char(arena, val->as.char_);
    case EL_PP_TYPE_STR:   return _el_pp_new_str(arena, val->as.str_);
    case EL_PP_TYPE_TOK:   return _el_pp_new_tok(arena, val->as.tok_);
    case EL_PP_TYPE_LIST: {
        ElPpValue** new_values = NULL;
        if (val->as.list_.count != 0) {
            new_values = EL_DYNARENA_NEW_ARR(arena, ElPpValue*, val->as.list_.count);
            for (usize i = 0; i < val->as.list_.count; ++i) {
                new_values[i] = _el_pp_value_clone(arena, val->as.list_.values[i]);
            }
        }
        return _el_pp_new_list(arena, (ElPpList) { new_values, val->as.list_.count });
    }
    }
    EL_UNREACHABLE_ENUM_VAL(ElPpType, val->type);
}

/////// constructors ///////
ElPpValue* _el_pp_new_null(ElDynArena* arena) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElPpValue, {
        .type = EL_PP_TYPE_NULL,
    });
}

ElPpValue* _el_pp_new_int(ElDynArena* arena, int64_t val) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElPpValue, {
        .type = EL_PP_TYPE_INT,
        .as.int_ = val,
    });
}

ElPpValue* _el_pp_new_float(ElDynArena* arena, double val) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElPpValue, {
        .type = EL_PP_TYPE_FLOAT,
        .as.float_ = val,
    });
}

ElPpValue* _el_pp_new_bool(ElDynArena* arena, bool val) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElPpValue, {
        .type = EL_PP_TYPE_BOOL,
        .as.bool_ = val,
    });
}

ElPpValue* _el_pp_new_char(ElDynArena* arena, char val) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElPpValue, {
        .type = EL_PP_TYPE_CHAR,
        .as.char_ = val,
    });
}

ElPpValue* _el_pp_new_str(ElDynArena* arena, ElStringView val) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElPpValue, {
        .type = EL_PP_TYPE_STR,
        .as.str_ = el_dynarena_clone_sv(arena, val),
    });
}

ElPpValue* _el_pp_new_tok(ElDynArena* arena, ElToken tok) {
    tok.lexeme = el_dynarena_clone_sv(arena, tok.lexeme);
    return EL_DYNARENA_NEW_STRUCT(arena, ElPpValue, {
        .type = EL_PP_TYPE_TOK,
        .as.tok_ = tok,
    });
}

ElPpValue* _el_pp_new_list(ElDynArena* arena, ElPpList list) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElPpValue, {
        .type = EL_PP_TYPE_LIST,
        .as.list_ = list,
    });
}
