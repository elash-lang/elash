#pragma once

#include <elash/ast/common/ident.h>
#include <elash/ast/common/type.h>

#include <elash/ast/stmt/block.h>
#include <elash/util/dynarena.h>
#include <elash/srcdoc/span.h>

#include <elash/defs/int-types.h>

typedef struct ElAstTopLevelNode ElAstTopLevelNode;

typedef struct ElAstFuncParam ElAstFuncParam;
struct ElAstFuncParam {
    ElSourceSpan span;
    ElAstTypeNode* type;
    ElAstIdentNode* name;

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
    ElAstTypeNode* ret_type;
    ElAstIdentNode* name;
    ElAstFuncParamList params;
} ElAstFuncSignature;

ElAstFuncParam el_ast_func_param(ElSourceSpan span, ElAstTypeNode* type, ElAstIdentNode* name);
ElAstFuncParam* el_ast_new_func_param(ElDynArena* arena, ElSourceSpan span, ElAstTypeNode* type, ElAstIdentNode* name);

ElAstFuncParamList el_ast_make_func_param_list();
void el_ast_func_param_list_append(ElAstFuncParamList* list, ElAstFuncParam* param);

ElAstFuncSignature el_ast_func_signature(
    ElSourceSpan span, ElAstTypeNode* ret_type, ElAstIdentNode* name, ElAstFuncParamList params
);
