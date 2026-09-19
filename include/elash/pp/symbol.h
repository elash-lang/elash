#pragma once
#include <elash/pp/value.h>
#include <elash/lexer/tokbuf.h>

typedef enum ElPpSymbolKind {
    EL_PP_SYM_VAR,
    EL_PP_SYM_FUNC,
    //EL_PP_SYM_MACRO,
} ElPpSymbolKind;

typedef struct ElPpVarSym {
    ElPpValue* v;
    bool is_mutable;
    bool was_mutated;
} ElPpVarSym;

typedef struct ElPpFuncParam {
    ElStringView name;
    struct ElPpFuncParam* next;
} ElPpFuncParam;

typedef struct ElPpParamList {
    ElPpFuncParam* head;
    ElPpFuncParam* tail;
    usize count;
} ElPpParamList;

typedef struct ElPpFuncSym {
    ElPpParamList params;
    ElToken* body;
    usize body_len;
} ElPpFuncSym;

typedef struct ElPpSymbol {
    ElStringView   name;
    ElPpSymbolKind kind;
    ElSourceSpan defspan;
    bool is_public;
    union {
        ElPpVarSym  var;
        ElPpFuncSym func;
    } as;
} ElPpSymbol;

ElPpSymbol* _el_pp_new_sym_var(
    ElDynArena* arena, ElStringView name, ElSourceSpan defspan,
    ElPpValue* value, bool mut, bool is_public
);

#define _el_pp_new_sym_func(ARENA, NAME, DEFSPAN, IS_PUBLIC, ...) \
    _el_pp_new_sym_func_impl(                                     \
        (ARENA), (NAME), (DEFSPAN), (IS_PUBLIC),                  \
        (ElPpFuncSym) { __VA_ARGS__ }                             \
    )

ElPpSymbol* _el_pp_new_sym_func_impl(
    ElDynArena* arena, ElStringView name,
    ElSourceSpan defspan, bool is_public,
    ElPpFuncSym func
);

ElStringView _el_pp_sym_kind_to_string(ElPpSymbol* sym);
