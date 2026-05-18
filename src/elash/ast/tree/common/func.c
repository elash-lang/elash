#include <elash/ast/tree/common/func.h>
#include <stddef.h>

ElAstFuncParam el_ast_func_param(ElSourceSpan span, ElAstType* type, ElAstIdent* name) {
    return (ElAstFuncParam) {
        .span = span,
        .type = type,
        .name = name,
        .next = NULL,
        .prev = NULL,
    };
}

ElAstFuncParam* el_ast_new_func_param(ElDynArena* arena, ElSourceSpan span, ElAstType* type, ElAstIdent* name) {
    ElAstFuncParam* param = EL_DYNARENA_NEW(arena, ElAstFuncParam);
    *param = el_ast_func_param(span, type, name);
    return param;
}

ElAstFuncParamList el_ast_make_func_param_list() {
    return (ElAstFuncParamList) {
        .head = NULL,
        .tail = NULL,
        .count = 0,
    };
}

void el_ast_func_param_list_append(ElAstFuncParamList* list, ElAstFuncParam* param) {
    if (list->head == NULL) {
        list->head = param;
        list->tail = param;
    } else {
        list->tail->next = param;
        param->prev = list->tail;
        list->tail = param;
    }
    list->count++;
}

ElAstFuncSignature el_ast_func_signature(
    ElSourceSpan span, ElAstType* ret_type, ElAstIdent* name, ElAstFuncParamList params
) {
    return (ElAstFuncSignature) {
        .span = span,
        .ret_type = ret_type,
        .name = name,
        .params = params,
    };
}
