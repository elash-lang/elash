#include "parser-internals.h"

#include <elash/ast/tree/module.h>
#include <elash/diag/engine.h>

ElAstModule* el_parse_module(ElParser* parser) {
    ElSourceSpan module_span = EL_SRCSPAN_NULL;
    bool first = true;

    if (parser->current.type != EL_TT_EOF) {
        module_span = parser->current.span;
    }

    ElAstModule* module = el_ast_new_module(parser->aarena, module_span);

    while (parser->current.type != EL_TT_EOF) {
        ElAstDecl* node = el_parse_decl(parser);
        if (node == NULL) {
            continue;
        }

        el_ast_module_append(module, node);
        if (first) {
            module->span = node->span;
            first = false;
        } else {
            module->span = el_srcspan_merge(module->span, node->span);
        }
    }

    return module;
}
