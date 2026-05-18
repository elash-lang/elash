#include <elash/parser/parser.h>

#include <elash/diag/engine.h>

#include <elash/ast/tree/module.h>

ElAstModule* el_parser_parse_module(ElParser* parser) {
    ElSourceSpan module_span = EL_SOURCE_SPAN_NULL;
    bool first = true;

    if (parser->current.type != EL_TT_EOF) {
        module_span = parser->current.span;
    }

    ElAstModule* module = el_ast_new_module(parser->arena, module_span);

    while (parser->current.type != EL_TT_EOF) {
        ElAstTopLevel* node = el_parser_parse_toplevel(parser);
        if (node == NULL) {
            continue;
        }

        el_ast_module_append(module, node);
        if (first) {
            module->span = node->span;
            first = false;
        } else {
            module->span = el_source_span_merge(module->span, node->span);
        }
    }

    return module;
}
