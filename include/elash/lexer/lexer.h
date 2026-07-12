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
/// @details By default, skips whitespace, and allows UTF-8 identifiers.
#define EL_LEXER_FLAGS_DEFAULT (EL_LF_SKIP_WHITESPACE | EL_LF_ALLOW_UTF8_IDENTS)

typedef enum ElLexerContext {
    EL_LEXER_CTX_DEFAULT,
    EL_LEXER_CTX_PP,
} ElLexerContext;

typedef struct ElLexer {
    const ElSourceDocument* doc;
    ElSourceLocation current_loc;
    ElSourceLocation token_start_loc;
    ElLexerContext ctx;
    ElLexerContext prev_ctx;

    ElLexerErrorDetails last_err_details;
    ElLexerFlags flags;
} ElLexer;

ElLexerErrorCode el_lexer_init(ElLexer* lexer, const ElSourceDocument* doc, ElLexerFlags flags);

ElLexerErrorCode el_lexer_reset(ElLexer* lexer);
ElLexerErrorCode el_lexer_set_document(ElLexer* lexer, const ElSourceDocument* doc);

ElLexerErrorCode el_lexer_next_token(ElLexer* lexer, ElToken* out_tok);

ElLexerContext el_lexer_get_current_context(const ElLexer* lexer);
ElLexerContext el_lexer_get_previous_context(const ElLexer* lexer);
bool el_lexer_entered_context(const ElLexer* lexer, ElLexerContext context);
bool el_lexer_exited_context(const ElLexer* lexer, ElLexerContext context);

ElTokenStream el_lexer_as_token_stream(ElLexer* lexer);

////// IMPLEMENTATION DETAILS //////////////////////
// note: those functions are implementation
// details and should not be called directly
// outside internal el-lexer implementation.
ElLexerErrorCode _el_lexer_ret_token(ElLexer* lexer, ElTokenType type, ElToken* out_tok);
ElLexerErrorCode _el_lexer_ret_token_with_lexeme(ElLexer* lexer, ElTokenType type, ElStringView lexeme, ElToken* out_tok);

ElLexerErrorCode _el_lexer_lex_op2(ElLexer* lexer, char expect, ElTokenType single, ElTokenType dbl, ElToken* out);
ElLexerErrorCode _el_lexer_lex_op3(ElLexer* lexer, char expect1, char expect2, ElTokenType single, ElTokenType dbl, ElTokenType triple, ElToken* out);
///////////////////////////////////////////////////
