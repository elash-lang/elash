#pragma once

#include <elash/util/dynarena.h>
#include <elash/srcdoc/srcdoc.h>

typedef struct ElAstDecl ElAstDecl;
typedef struct ElAstType ElAstType;

typedef struct ElAstStructType {
    ElAstDecl* fields;
    usize count;
} ElAstStructType;

ElAstType* el_ast_new_type_struct(ElDynArena* arena, ElSourceSpan span, ElAstDecl* fields, usize count);
