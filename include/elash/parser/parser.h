#pragma once

#include <elash/lexer/tokstream.h>

#include <elash/ast/expr.h>
#include <elash/ast/stmt.h>
#include <elash/ast/toplevel.h>
#include <elash/ast/module.h>
#include <elash/ast/common/type.h>

#include <elash/parser/error.h>

typedef struct ElParser {
    ElTokenStream tokens;
    ElDiagEngine* engine;
    ElDynArena* arena;

    ElToken current;
    ElToken lookahead;
    bool has_lookahead;

    ElParserErrorDetails last_err_details;
} ElParser;

void el_parser_init(ElParser* parser, ElTokenStream tokens, ElDiagEngine* engine, ElDynArena* arena);

ElParserErrorCode el_parser_advance(ElParser* parser);
ElParserErrorCode el_parser_expect(ElParser* parser, ElTokenType type);

bool    el_parser_match(ElParser* parser, ElTokenType type); 
bool    el_parser_check(ElParser* parser, ElTokenType type);
ElToken el_parser_peek(ElParser* parser);

ElParserErrorCode _el_parser_parse_ident(ElParser* parser, ElAstIdentNode** out);
ElParserErrorCode _el_parser_parse_type(ElParser* parser, ElAstTypeNode** out);

ElParserErrorCode _el_parser_parse_expression(ElParser* parser, ElAstExprNode** out);
ElParserErrorCode _el_parser_parse_primary(ElParser* parser, ElAstExprNode** out);
ElParserErrorCode _el_parser_parse_postfix(ElParser* parser, ElAstExprNode** out);
ElParserErrorCode _el_parser_parse_unary(ElParser* parser, ElAstExprNode** out);
ElParserErrorCode _el_parser_parse_multiplicative(ElParser* parser, ElAstExprNode** out);
ElParserErrorCode _el_parser_parse_additive(ElParser* parser, ElAstExprNode** out);
ElParserErrorCode _el_parser_parse_shift(ElParser* parser, ElAstExprNode** out);
ElParserErrorCode _el_parser_parse_relational(ElParser* parser, ElAstExprNode** out);
ElParserErrorCode _el_parser_parse_equality(ElParser* parser, ElAstExprNode** out);
ElParserErrorCode _el_parser_parse_bitwise_and(ElParser* parser, ElAstExprNode** out);
ElParserErrorCode _el_parser_parse_bitwise_xor(ElParser* parser, ElAstExprNode** out);
ElParserErrorCode _el_parser_parse_bitwise_or(ElParser* parser, ElAstExprNode** out);
ElParserErrorCode _el_parser_parse_logical_and(ElParser* parser, ElAstExprNode** out);
ElParserErrorCode _el_parser_parse_logical_or(ElParser* parser, ElAstExprNode** out);

ElParserErrorCode _el_parser_parse_return(ElParser* parser, ElToken return_tok, ElAstStmtNode** out);
ElParserErrorCode _el_parser_parse_if(ElParser* parser, ElToken if_tok, ElAstStmtNode** out);
ElParserErrorCode _el_parser_parse_expr_stmt(ElParser* parser, ElAstStmtNode** out);
ElParserErrorCode _el_parser_parse_block(ElParser* parser, ElToken lbrace_tok, ElAstStmtNode** out);
ElParserErrorCode _el_parser_parse_stmt(ElParser* parser, ElAstStmtNode** out);

ElParserErrorCode _el_parser_parse_func_def(ElParser* parser, ElAstTopLevelNode** out);
ElParserErrorCode _el_parser_parse_toplevel(ElParser* parser, ElAstTopLevelNode** out);

ElParserErrorCode _el_parser_parse_module(ElParser* parser, ElAstModuleNode** out);

ElParserErrorCode el_parser_parse(ElParser* parser, ElAstModuleNode** out);
