#include "parser-internals.h"

#include <elash/ast/tree/toi.h>

static ElAstToI* _parse_toi_internal(ElParser* parser) {
    if (el_parser_check(parser, EL_TT_LBRACE)) {
        ElAstInit* init = el_parse_init(parser);
        if (init == NULL) return NULL;
        return el_ast_new_toi_init(parser->aarena, init);
    }

    ElAstToE* toe = el_parse_toe(parser);
    return el_ast_toi_from_toe(parser->aarena, toe);
}

ElAstToI* el_parse_toi(ElParser* parser) {
    el_prof_begin_sub(parser->prof, parser->pss_toi);
    ElAstToI* result = _parse_toi_internal(parser);
    el_prof_finish_sub(parser->prof, parser->pss_toi);
    return result;
}
