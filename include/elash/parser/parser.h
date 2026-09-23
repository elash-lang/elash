#pragma once

#include <elash/parser/recovery.h>

#include <elash/defs/int-types.h>
#include <elash/lexer/tokstream.h>

#include <elash/lexer/tokque.h>

#include <elash/ast/tree/expr.h>
#include <elash/ast/tree/stmt.h>
#include <elash/ast/tree/init.h>
#include <elash/ast/tree/decl.h>
#include <elash/ast/tree/module.h>
#include <elash/ast/tree/type.h>
#include <elash/ast/tree/toe.h>
#include <elash/ast/tree/toi.h>

#include <elash/diag/engine.h>
#include <elash/prof/prof.h>

typedef struct ElParser {
    ElTokenStream tokens;
    ElDiagEngine* diag;

    ElDynArena* farena;
    ElDynArena* aarena;

    ElProfState* prof;
    ElProfSubstage
        *pss_expr,
        *pss_stmt,
        *pss_decl,
        *pss_type,
        *pss_init,
        *pss_toi,
        *pss_toe;

    ElToken current;
    ElTokenQueue lookahead;
} ElParser;

void el_parser_init(
    ElParser* parser, ElTokenStream tokens, ElDiagEngine* engine,
    ElDynArena* farena, ElDynArena* aarena, ElProfState* prof
);
void el_parser_destroy(ElParser* parser);

bool el_parser_has_errs(const ElParser* parser);
uint el_parser_error_count(const ElParser* parser);
bool el_parser_had_new_errors(const ElParser* parser, uint error_count_before);

ElToken el_parser_advance(ElParser* parser);
ElToken el_parser_expect(ElParser* parser, ElTokenType type);

bool    el_parser_match(ElParser* parser, ElTokenType type);
bool    el_parser_check(ElParser* parser, ElTokenType type);
ElToken el_parser_peek(ElParser* parser);
ElToken el_parser_peek_at(ElParser* parser, usize n);

ElAstToE*      el_parse_toe(ElParser* parser);
ElAstToI*      el_parse_toi(ElParser* parser);
ElAstDecl*     el_parse_decl(ElParser* parser);
ElAstInit*     el_parse_init(ElParser* parser);
ElAstExpr*     el_parse_expr(ElParser* parser);
ElAstStmt*     el_parse_stmt(ElParser* parser);
ElAstModule*   el_parse_module(ElParser* parser);
