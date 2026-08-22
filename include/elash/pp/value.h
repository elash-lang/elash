#pragma once

#include <elash/defs/sv.h>
#include <elash/util/strbuf.h>
#include <elash/lexer/tokbuf.h>

typedef enum ElPpType {
    EL_PP_TYPE_NULL,
    EL_PP_TYPE_INT,
    EL_PP_TYPE_BOOL,
    EL_PP_TYPE_FLOAT,
    EL_PP_TYPE_CHAR,
    EL_PP_TYPE_LIST,
    EL_PP_TYPE_STR,
    EL_PP_TYPE_TOK,
} ElPpType;

typedef struct ElPpValue ElPpValue;
typedef struct ElPreproc ElPreproc;

typedef struct ElPpList {
    ElPpValue** values;
    usize count;
} ElPpList;

typedef struct ElPpValue {
    // TODO: use big ints / big floats
    union {
        int64_t      int_;   // EL_PP_TYPE_INT
        double       float_; // EL_PP_TYPE_FLOAT
        bool         bool_;  // EL_PP_TYPE_BOOL
        char         char_;  // EL_PP_TYPE_CHAR
        ElPpList     list_;  // EL_PP_TYPE_LIST
        ElStringView str_;   // EL_PP_TYPE_STR
        ElToken      tok_;   // EL_PP_TYPE_TOK
    } as;
    ElPpType type;
} ElPpValue;

ElStringView _el_pp_type_name(ElPpType type);

bool _el_pp_value_eq(ElPreproc* pp, ElPpValue* lhs, ElPpValue* rhs, ElSourceSpan span);
bool _el_pp_value_lt(ElPreproc* pp, ElPpValue* lhs, ElPpValue* rhs, ElSourceSpan span);
bool _el_pp_value_gt(ElPreproc* pp, ElPpValue* lhs, ElPpValue* rhs, ElSourceSpan span);

ElPpValue* _el_pp_strcat(ElPreproc* pp, ElPpValue* lhs, ElPpValue* rhs);
ElPpValue* _el_pp_listcat(ElPreproc* pp, ElPpValue* lhs, ElPpValue* rhs);

ElPpValue* _el_pp_value_clone(ElDynArena* arena, ElPpValue* value);

ElPpValue* _el_pp_new_null(ElDynArena* arena);
ElPpValue* _el_pp_new_int(ElDynArena* arena, int64_t val);
ElPpValue* _el_pp_new_float(ElDynArena* arena, double val);
ElPpValue* _el_pp_new_bool(ElDynArena* arena, bool val);
ElPpValue* _el_pp_new_char(ElDynArena* arena, char val);
ElPpValue* _el_pp_new_str(ElDynArena* arena, ElStringView val);
ElPpValue* _el_pp_new_tok(ElDynArena* arena, ElToken tok);
ElPpValue* _el_pp_new_list(ElDynArena* arena, ElPpList list);
