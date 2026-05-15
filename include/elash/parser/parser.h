#pragma once

#include <elash/parser/recovery.h>

#include <elash/defs/int-types.h>
#include <elash/lexer/tokstream.h>

#include <elash/ast/tree/expr.h>
#include <elash/ast/tree/stmt.h>
#include <elash/ast/tree/toplevel.h>
#include <elash/ast/tree/module.h>
#include <elash/ast/tree/common/type.h>

#include <elash/diag/engine.h>

typedef struct ElParser {
    ElTokenStream tokens;
    ElDiagEngine* diag;
    ElDynArena* arena;

    ElToken current;
    ElToken lookahead;
    bool has_lookahead;
} ElParser;

void el_parser_init(ElParser* parser, ElTokenStream tokens, ElDiagEngine* engine, ElDynArena* arena);

bool el_parser_has_errs(const ElParser* parser);
uint el_parser_error_count(const ElParser* parser);
bool el_parser_had_new_errors(const ElParser* parser, uint error_count_before);

void el_parser_advance(ElParser* parser);
void el_parser_expect(ElParser* parser, ElTokenType type);

bool    el_parser_match(ElParser* parser, ElTokenType type);
bool    el_parser_check(ElParser* parser, ElTokenType type);
ElToken el_parser_peek(ElParser* parser);

void _el_parser_report_expected(ElParser* parser, ElTokenType expected);
void _el_parser_report_unexpected(ElParser* parser, ElToken tok);

ElAstStmtNode*  _el_parser_parse_block(ElParser* parser, ElToken lbrace_tok);
ElAstIdentNode* _el_parser_parse_ident(ElParser* parser);
ElAstTypeNode*  _el_parser_parse_type(ElParser* parser);

ElAstExprNode*     el_parser_parse_expr(ElParser* parser);
ElAstStmtNode*     el_parser_parse_stmt(ElParser* parser);
ElAstTopLevelNode* el_parser_parse_toplevel(ElParser* parser);
ElAstModuleNode*   el_parser_parse_module(ElParser* parser);
