#include <elash/parser/parser.h>
#include <elash/parser/utility.h>

#include <elash/ast/tree/module.h>

ElParserErrorCode _el_parser_parse_module(ElParser* parser, ElAstModuleNode** out) {
    ElSourceSpan module_span = EL_SOURCE_SPAN_NULL;
    bool first = true;

    // We can't easily know the full span until we parse everything.
    // If there are no nodes, the span is NULL.
    // Otherwise, it merges from the first node to the last.

    // Actually, a better way: start with the first token's span.
    if (parser->current.type != EL_TT_EOF) {
        module_span = parser->current.span;
    }

    ElAstModuleNode* module = el_ast_new_module(parser->arena, module_span);

    while (parser->current.type != EL_TT_EOF) {
        ElAstTopLevelNode* node = NULL;
        ElParserErrorCode err = _el_parser_parse_toplevel(parser, &node);
        if (err != EL_PARSER_ERR_OK) return err;

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

    *out = module;

    return _el_parser_ret_ok(parser);
}
