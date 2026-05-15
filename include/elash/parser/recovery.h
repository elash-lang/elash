#pragma once

#include <elash/lexer/token.h>

typedef struct ElParser ElParser;
typedef enum ElParserSyncKind {
    EL_PARSER_SYNC_STMT,
    EL_PARSER_SYNC_BLOCK,
    EL_PARSER_SYNC_DECL,
    EL_PARSER_SYNC_EXPR,
} ElParserSyncKind;

// this functions just always returns NULL.
// i know it IS UGLY. but it allows writing
// if (...)
//     return el_parser_sync(parser, EL_PARSER_SYNC_EXPR);
//
// instead of
// if (...) {
//     el_parser_sync(parser, EL_PARSER_SYNC_EXPR);
//     return NULL;
// }
//
// and i hate boilerplate! (:
void* el_parser_sync(ElParser* parser, ElParserSyncKind kind);

void el_parser_skip_to(ElParser* parser, ElTokenType type);
void el_parser_skip_balanced_parens(ElParser* parser);
void el_parser_skip_balanced_braces(ElParser* parser);
