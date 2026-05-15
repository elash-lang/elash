#include <elash/parser/parser.h>

#include <elash/diag/engine.h>

#include <elash/ast/tree/module.h>

ElAstModuleNode* el_parser_parse_module(ElParser* parser) {
    ElSourceSpan module_span = EL_SOURCE_SPAN_NULL;
    bool first = true;

    if (parser->current.type != EL_TT_EOF) {
        module_span = parser->current.span;
    }

    ElAstModuleNode* module = el_ast_new_module(parser->arena, module_span);

    while (parser->current.type != EL_TT_EOF) {
        ElAstTopLevelNode* node = el_parser_parse_toplevel(parser);
        if (el_parser_has_errs(parser)) return module;

        if (node) {
            el_ast_module_append(module, node);
            if (first) {
                module->span = node->span;
                first = false;
            } else {
                module->span = el_source_span_merge(module->span, node->span);
            }
        }
    }

    return module;
}
