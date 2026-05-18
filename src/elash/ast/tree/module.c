#include <elash/ast/tree/module.h>
#include <elash/ast/tree/expr.h>

ElAstModule* el_ast_new_module(ElDynArena* arena, ElSourceSpan span) {
    ElAstModule* node = EL_DYNARENA_NEW_ZEROED(arena, ElAstModule);
    node->span = span;
    return node;
}

void el_ast_module_append(ElAstModule* module, ElAstTopLevel* node) {
    if (module->head == NULL) {
        module->head = node;
        module->tail = node;
    } else {
        module->tail->next = node;
        module->tail = node;
    }
    module->count++;
}
