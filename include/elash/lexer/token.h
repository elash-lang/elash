#pragma once

#include <elash/source/span.h>
#include <elash/defs/sv.h>

#include <elash/util/strbuf.h>

#include <stdio.h>

typedef enum ElTokenType {
    EL_TT_EOF = 0,

    EL_TT_IDENT,                // somename

    EL_TT_INT_LITERAL,          // 123
    EL_TT_FLOAT_LITERAL,        // 3.14
    EL_TT_CHAR_LITERAL,         // 'x'
    EL_TT_STRING_LITERAL,       // "Hello world!"

    // Group enumerator - DO NOT USE as a token type. Marks the start of the keyword group.
    // Used internally for bounds checking or iteration over keyword tokens.
    _EL_TT_GROUP_BEGIN_KEYWORDS,

    EL_TT_TRUE_LITERAL,         // true
    EL_TT_FALSE_LITERAL,        // false
    EL_TT_NULL_LITERAL,         // null

    EL_TT_KW_IF,                // if
    EL_TT_KW_ELSE,              // else
    EL_TT_KW_SWITCH,            // switch

    EL_TT_KW_WHILE,             // while
    EL_TT_KW_FOR,               // for
    EL_TT_KW_DO,                // do

    EL_TT_KW_CONTINUE,          // continue
    EL_TT_KW_DEFAULT,           // default
    EL_TT_KW_RETURN,            // return
    EL_TT_KW_BREAK,             // break
    EL_TT_KW_CASE,              // case
    EL_TT_KW_GOTO,              // goto

    EL_TT_KW_EXTERN,            // extern
    EL_TT_KW_INTERNAL,          // internal
    EL_TT_KW_STATIC,            // global

    EL_TT_KW_VOLATILE,          // volatile
    EL_TT_KW_CONST,             // ronly
    EL_TT_KW_WONLY,             // wonly

    EL_TT_KW_AS,                // as

    EL_TT_KW_ENUM,              // enum
    EL_TT_KW_UNION,             // union
    EL_TT_KW_STRUCT,            // struct
    EL_TT_KW_TYPEDEF,           // typedef
    EL_TT_KW_SIZEOF,            // sizeof
    EL_TT_KW_ALIAS,             // alias

    // Group enumerator - DO NOT USE as a token type. Marks the end of the keyword group.
    // Used internally for bounds checking or iteration over keyword tokens.
    _EL_TT_GROUP_END_KEYWORDS,

    EL_TT_PLUS,                 // +
    EL_TT_MINUS,                // -
    EL_TT_STAR,                 // *
    EL_TT_SLASH,                // /
    EL_TT_PERCENT,              // %
    EL_TT_CARET,                // ^

    EL_TT_INC,                  // ++
    EL_TT_DEC,                  // --

    EL_TT_ASSIGN,               // =
    EL_TT_ADD_ASSIGN,           // +=
    EL_TT_SUB_ASSIGN,           // -=
    EL_TT_MUL_ASSIGN,           // *=
    EL_TT_DIV_ASSIGN,           // /=
    EL_TT_MOD_ASSIGN,           // %=

    EL_TT_EQL,                  // ==
    EL_TT_NEQ,                  // !=
    EL_TT_LT,                   // <
    EL_TT_LTE,                  // <=
    EL_TT_GT,                   // >
    EL_TT_GTE,                  // >=

    EL_TT_LOGICAL_AND,          // &&
    EL_TT_LOGICAL_OR,           // ||
    EL_TT_LOGICAL_NOT,          // !
    EL_TT_LOGICAL_IMP,          // =>

    EL_TT_LOGICAL_AND_ASSIGN,   // &&=
    EL_TT_LOGICAL_OR_ASSIGN,    // ||=
    EL_TT_LOGICAL_IMP_ASSIGN,   // =>=

    EL_TT_BITWISE_AND,          // &
    EL_TT_BITWISE_OR,           // |
    EL_TT_BITWISE_XOR,          // <>
    EL_TT_BITWISE_NOT,          // ~
    EL_TT_BITWISE_IMP,          // ~>

    EL_TT_BITWISE_AND_ASSIGN,   // &=
    EL_TT_BITWISE_OR_ASSIGN,    // |=
    EL_TT_BITWISE_XOR_ASSIGN,   // <>=
    EL_TT_BITWISE_IMP_ASSIGN,   // ~>=

    EL_TT_SHL,                  // <<
    EL_TT_SHR,                  // >>
    EL_TT_SHL_ASSIGN,           // <<=
    EL_TT_SHR_ASSIGN,           // >>=

    EL_TT_LPAREN,               // (
    EL_TT_RPAREN,               // )
    EL_TT_LBRACKET,             // [
    EL_TT_RBRACKET,             // ]
    EL_TT_LBRACE,               // {
    EL_TT_RBRACE,               // }

    EL_TT_SEMICOLON,            // ;
    EL_TT_COLON,                // :
    EL_TT_DOUBLECOLON,          // ::
    EL_TT_COMMA,                // ,
    EL_TT_DOT,                  // .

    EL_TT_HASH,                 // #

    EL_TT_ELLIPSIS,             // ...
    EL_TT_LINE_COMMENT,         // //
    EL_TT_BLOCK_COMMENT,        // /* */

    EL_TT_WHITESPACE,           // (space) , \t
    EL_TT_NEWLINE,              // \n

    EL_TT_UNKNOWN,
    _EL_TT_COUNT,
} ElTokenType;

typedef struct ElToken {
    ElTokenType  type;
    ElStringView lexeme;
    ElSourceSpan span;
} ElToken;

bool el_token_type_is_keyword(ElTokenType tt);

ElStringView el_token_type_to_string(ElTokenType tt);
ElStringView el_token_type_format(ElTokenType tt);

/// @brief Format a token into a provided buffer.
/// @param tok Token to format.
/// @param n Size of the destination buffer.
/// @param buf Output buffer (must have at least `n` bytes).
/// @return Number of bytes written (excluding null terminator).
usize el_token_debug_format(const ElToken* tok, usize n, char buf[static n]);

/// @brief Convert a token into a newly allocated string representation.
/// @param tok Token to convert.
/// @param out Pointer to store the allocated string.
/// @return Length of the resulting string (excluding null terminator).
usize el_token_to_debug_string(const ElToken* tok, char** out);

/// @brief Print a token to a file stream.
/// @param tok Token to print.
/// @param out File stream to write to.
/// @return Number of bytes written.
usize el_token_print(const ElToken* tok, FILE* out);

/// @brief Convert a token into its raw string representation and append to a string buffer.
/// @param tok Token to convert.
/// @param sb String buffer to append to.
/// @return True on success, false on memory allocation failure.
bool el_token_to_raw_string(const ElToken* tok, ElStringBuf* sb, bool whitespace);
