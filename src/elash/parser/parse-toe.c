#include "parser-internals.h"

#include <elash/ast/tree/toe.h>

static ElAstToE* _parse_toe_internal(ElParser* parser) {
    if (_el_parser_is_complex_expr(parser)) {
        ElAstExpr* expr = el_parse_expr(parser);
        if (expr == NULL) return NULL;
        return el_ast_new_toe_expr(parser->aarena, expr);
    }

    // Type { ... }
    if (_el_parser_is_type_literal(parser)) {
        ElAstExpr* expr = _el_parse_postfix(parser);
        if (expr == NULL) return NULL;
        return el_ast_new_toe_expr(parser->aarena, expr);
    }

    return _el_parser_toe_from_ambig(parser, _el_parse_ambig(parser));
}

ElAstToE* el_parse_toe(ElParser* parser) {
    el_prof_begin_sub(parser->prof, parser->pss_toe);
    ElAstToE* result = _parse_toe_internal(parser);
    el_prof_finish_sub(parser->prof, parser->pss_toe);
    return result;
}
