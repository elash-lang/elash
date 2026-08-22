#pragma once

#include <stddef.h>

#include <elash/defs/sv.h>

#include <elash/lexer/error.h>
#include <elash/lexer/token.h>
#include <elash/lexer/tokstream.h>

/// @enum ElLexerFlags
/// @brief Represents configuration flags controlling lexer behavior.
/// These flags modify how the lexer processes the input stream. They can be combined using bitwise OR.
/// @note Unused bits are reserved for future extensions.
typedef enum ElLexerFlags {
    EL_LF_NONE = 0, ///< No flags.

    EL_LF_ALLOW_UNTERM = 1 << 0, ///< Allows unterminated strings/chars at the end of the input.
    EL_LF_ALLOW_UTF8_IDENTS = 1 << 1, ///< Allows idents with non-ascii characters.
    EL_LF_SKIP_COMMENTS = 1 << 2, ///< Skips all comments.
    EL_LF_SKIP_WHITESPACE = 1 << 3, ///< Skips all whitespaces.
    EL_LF_SKIP_UNKNOWN = 1 << 4, ///< Skips unknown tokens instanted of returning an error.
} ElLexerFlags;

/// @def EL_LEXER_FLAGS_DEFAULT
/// @brief Default lexer configuration flags.
/// @details By default, allows UTF-8 identifiers.
#define EL_LEXER_FLAGS_DEFAULT (EL_LF_ALLOW_UTF8_IDENTS)

typedef struct ElLexer {
    const ElSourceDocument* doc;
    ElSourceLocation current_loc;
    ElSourceLocation token_start_loc;

    ElLexerErrorDetails last_err_details;
    ElLexerFlags flags;
} ElLexer;

ElLexerErrorCode el_lexer_init(ElLexer* lexer, const ElSourceDocument* doc, ElLexerFlags flags);

ElLexerErrorCode el_lexer_reset(ElLexer* lexer);
ElLexerErrorCode el_lexer_set_document(ElLexer* lexer, const ElSourceDocument* doc);

ElLexerErrorCode el_lexer_next_token(ElLexer* lexer, ElToken* out_tok);
ElTokenStream    el_lexer_as_token_stream(ElLexer* lexer);
