#pragma once

#include <elash/parser/recovery.h>

#include <elash/defs/int-types.h>
#include <elash/lexer/tokstream.h>

#include <elash/lexer/tokque.h>

#include <elash/ast/tree/expr.h>
#include <elash/ast/tree/stmt.h>
#include <elash/ast/tree/toplevel.h>
#include <elash/ast/tree/module.h>
#include <elash/ast/tree/common/type.h>
#include <elash/ast/tree/common/init.h>

#include <elash/diag/engine.h>

typedef struct ElParser {
    ElTokenStream tokens;
    ElDiagEngine* diag;
    ElDynArena* arena;

    ElToken current;
    ElTokenQueue lookahead;
} ElParser;

void el_parser_init(ElParser* parser, ElTokenStream tokens, ElDiagEngine* engine, ElDynArena* arena);
void el_parser_destroy(ElParser* parser);

bool el_parser_has_errs(const ElParser* parser);
uint el_parser_error_count(const ElParser* parser);
bool el_parser_had_new_errors(const ElParser* parser, uint error_count_before);

void el_parser_advance(ElParser* parser);
void el_parser_expect(ElParser* parser, ElTokenType type);

bool    el_parser_match(ElParser* parser, ElTokenType type);
bool    el_parser_check(ElParser* parser, ElTokenType type);
ElToken el_parser_peek(ElParser* parser);
ElToken el_parser_peek_at(ElParser* parser, usize n);

void _el_parser_report_expected(ElParser* parser, ElTokenType expected);
void _el_parser_report_unexpected(ElParser* parser, ElToken tok);

bool _el_parser_lookahead_skip_type(ElParser* parser, usize* idx);

ElAstStmt*  _el_parser_parse_block(ElParser* parser, ElToken lbrace_tok);
ElAstIdent* _el_parser_parse_ident(ElParser* parser);
ElAstType*  _el_parser_parse_type(ElParser* parser);

ElAstInit*     el_parser_parse_init(ElParser* parser);
ElAstExpr*     el_parser_parse_expr(ElParser* parser);
ElAstStmt*     el_parser_parse_stmt(ElParser* parser);
ElAstTopLevel* el_parser_parse_toplevel(ElParser* parser);
ElAstModule*   el_parser_parse_module(ElParser* parser);
