#include <elash/ast/tree/module.h>
#include <elash/ast/tree/expr.h>

ElAstModuleNode* el_ast_new_module(ElDynArena* arena, ElSourceSpan span) {
    ElAstModuleNode* node = EL_DYNARENA_NEW_ZEROED(arena, ElAstModuleNode);
    node->span = span;
    return node;
}

void el_ast_module_append(ElAstModuleNode* module, ElAstTopLevelNode* node) {
    if (module->head == NULL) {
        module->head = node;
        module->tail = node;
    } else {
        module->tail->next = node;
        module->tail = node;
    }
    module->count++;
}
