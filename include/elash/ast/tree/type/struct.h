#pragma once

#include <elash/util/dynarena.h>
#include <elash/srcdoc/srcdoc.h>
#include <elash/ast/tree/common/ident.h>

typedef struct ElAstType ElAstType;

typedef struct ElAstStructField ElAstStructField;
struct ElAstStructField {
    ElSourceSpan span;
    ElAstIdent*  name;
    ElAstType*   type;
    ElAstStructField* next;
};

typedef struct ElAstStructType {
    ElAstStructField* fields;
    usize count;
} ElAstStructType;

ElAstType* el_ast_new_type_struct(ElDynArena* arena, ElSourceSpan span, ElAstStructField* fields, usize count);
void el_ast_struct_add_field(ElAstStructField** head, ElAstStructField** tail, ElAstStructField* field);
