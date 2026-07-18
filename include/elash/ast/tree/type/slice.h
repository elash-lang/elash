#pragma once

#include <elash/util/dynarena.h>
#include <elash/srcdoc/srcdoc.h>

typedef struct ElAstExpr ElAstExpr;
typedef struct ElAstType ElAstType;

typedef struct ElAstSliceType {
    ElAstType* base;
    bool is_raw;
} ElAstSliceType;

ElAstType* el_ast_new_type_slice(ElDynArena* arena, ElSourceSpan span, ElAstType* base, bool is_raw);
