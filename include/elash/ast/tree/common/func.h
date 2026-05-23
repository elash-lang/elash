#pragma once

#include <elash/ast/tree/common/ident.h>
#include <elash/ast/tree/common/block.h>
#include <elash/ast/tree/common/type.h>

#include <elash/util/dynarena.h>
#include <elash/srcdoc/span.h>

#include <elash/defs/int-types.h>

typedef struct ElAstTopLevel ElAstTopLevel;

typedef struct ElAstFuncParam ElAstFuncParam;
struct ElAstFuncParam {
    ElSourceSpan span;
    ElAstType* type;
    ElAstIdent* name;

    ElAstFuncParam* next;
    ElAstFuncParam* prev;
};

typedef struct ElAstFuncParamList {
    ElAstFuncParam* head;
    ElAstFuncParam* tail;
    usize count;
} ElAstFuncParamList;

typedef struct ElAstFuncSignature {
    ElSourceSpan span;
    ElAstType* ret_type;
    ElAstIdent* name;
    ElAstFuncParamList params;
} ElAstFuncSignature;

ElAstFuncParam el_ast_func_param(ElSourceSpan span, ElAstType* type, ElAstIdent* name);
ElAstFuncParam* el_ast_new_func_param(ElDynArena* arena, ElSourceSpan span, ElAstType* type, ElAstIdent* name);

ElAstFuncParamList el_ast_make_func_param_list();
void el_ast_func_param_list_append(ElAstFuncParamList* list, ElAstFuncParam* param);

ElAstFuncSignature el_ast_func_signature(
    ElSourceSpan span, ElAstType* ret_type, ElAstIdent* name, ElAstFuncParamList params
);
