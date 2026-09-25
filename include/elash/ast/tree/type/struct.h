#pragma once

#include <elash/sema/mutability.h>
#include <elash/util/dynarena.h>
#include <elash/source/doc.h>

typedef struct ElAstDecl ElAstDecl;
typedef struct ElAstType ElAstType;

typedef struct ElAstStructType {
    ElAstDecl* fields;
    usize count;
} ElAstStructType;

ElAstType* el_ast_new_type_struct(ElDynArena* arena, ElSourceSpan span, ElMutabilitySpec mut, ElAstDecl* fields, usize count);
