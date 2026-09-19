#include <elash/lexer/lexer.h>
#include <elash/lexer/token.h>
#include <elash/lexer/macros.h>

#include <elash/source/doc.h>
#include <elash/diag/handle.h>
#include <elash/prof/prof.h>
#include <elash/defs/sv.h>

#include <ctype.h>
#include <stdbool.h>

static inline char peek(const ElLexer* lexer) {
    ElStringView input = el_srcdoc_content(lexer->doc);
    if (lexer->current_loc.offset >= input.len)
        return '\0';

    return input.data[lexer->current_loc.offset];
}

static inline char peek_next(const ElLexer* lexer) {
    ElStringView input = el_srcdoc_content(lexer->doc);
    if (lexer->current_loc.offset + 1 >= input.len)
        return '\0';

    return input.data[lexer->current_loc.offset + 1];
}

static inline char next(ElLexer* lexer) {
    ElStringView input = el_srcdoc_content(lexer->doc);
    if (lexer->current_loc.offset >= input.len)
        return '\0';

    char c = input.data[lexer->current_loc.offset++];
    lexer->current_loc.column++;

    if (c == '\r') {
        if (peek(lexer) == '\n') {
            c = input.data[lexer->current_loc.offset++];
        }
        lexer->current_loc.line++;
        lexer->current_loc.column = 0;
    } else if (c == '\n') {
        lexer->current_loc.line++;
        lexer->current_loc.column = 0;
    }

    return c;
}

static ElLexerStatus ret_token(ElLexer* lexer, ElTokenType type, ElToken* out_tok) {
    lexer->last_err_details = EL_LEXER_RESULT_SUCCESS;
    out_tok->type = type;
    out_tok->span = el_srcspan_make(lexer->doc, lexer->token_start_loc, lexer->current_loc);
    out_tok->lexeme = el_srcspan_to_sv(out_tok->span);
    return EL_LEXERR_SUCCESS;
}
static ElLexerStatus ret_token_with_lexeme(ElLexer* lexer, ElTokenType type, ElStringView lexeme, ElToken* out_tok) {
    lexer->last_err_details = EL_LEXER_RESULT_SUCCESS;
    out_tok->type = type;
    out_tok->span = el_srcspan_make(lexer->doc, lexer->token_start_loc, lexer->current_loc);
    out_tok->lexeme = lexeme;
    return EL_LEXERR_SUCCESS;
}

static inline ElStringView el_make_lexeme_from_token_start(ElLexer* lexer) {
    return el_sv_slice(el_srcdoc_content(lexer->doc), lexer->token_start_loc.offset, lexer->current_loc.offset);
}
static inline ElLexerStatus ret_tok_with_lexeme_auto(ElLexer* lexer, ElTokenType t, ElToken* out) {
    return ret_token_with_lexeme(lexer, t, el_make_lexeme_from_token_start(lexer), out);
}

ElLexerStatus el_lexer_init(ElLexer* lexer, const ElSourceDocument* doc, ElLexerFlags flags) {
    return el_lexer_init_prof(lexer, doc, flags, NULL, NULL);
}

ElLexerStatus el_lexer_init_prof(
    ElLexer* lexer, const ElSourceDocument* doc, ElLexerFlags flags,
    ElProfState* prof, ElProfStage* prof_stage
) {
    lexer->doc = doc;
    lexer->current_loc = EL_SOURCE_LOC_ZERO;
    lexer->token_start_loc = EL_SOURCE_LOC_ZERO;
    lexer->last_err_details = EL_LEXER_RESULT_SUCCESS;
    lexer->flags = flags;

    lexer->prof = prof;
    lexer->prof_stage = prof_stage;

    return EL_LEXERR_SUCCESS;
}

ElLexerStatus el_lexer_reset(ElLexer* lexer) {
    lexer->current_loc = EL_SOURCE_LOC_ZERO;
    lexer->token_start_loc = EL_SOURCE_LOC_ZERO;
    lexer->doc = NULL;
    lexer->last_err_details.code = EL_LEXERR_SUCCESS;
    return EL_LEXERR_SUCCESS;
}

ElLexerStatus el_lexer_set_document(ElLexer* lexer, const ElSourceDocument* doc) {
    ElLexerStatus err = el_lexer_reset(lexer);

    lexer->doc = doc;

    return err;
}

ElTokenType get_keyword_or_ident_type(ElStringView lexeme) {
    typedef struct StringToKeyword {
        ElStringView str;
        ElTokenType kwtype;
    } StringToKeyword;

    static StringToKeyword keywords[] = {
        { EL_SV("as"),       EL_TT_KW_AS         },
        { EL_SV("do"),       EL_TT_KW_DO         },
        { EL_SV("if"),       EL_TT_KW_IF         },
        { EL_SV("for"),      EL_TT_KW_FOR        },
        { EL_SV("case"),     EL_TT_KW_CASE       },
        { EL_SV("else"),     EL_TT_KW_ELSE       },
        { EL_SV("enum"),     EL_TT_KW_ENUM       },
        { EL_SV("goto"),     EL_TT_KW_GOTO       },
        { EL_SV("null"),     EL_TT_NULL_LITERAL  },
        { EL_SV("true"),     EL_TT_TRUE_LITERAL  },
        { EL_SV("break"),    EL_TT_KW_BREAK      },
        { EL_SV("const"),    EL_TT_KW_CONST      },
        { EL_SV("wonly"),    EL_TT_KW_WONLY      },
        { EL_SV("alias"),    EL_TT_KW_ALIAS      },
        { EL_SV("false"),    EL_TT_FALSE_LITERAL },
        { EL_SV("union"),    EL_TT_KW_UNION      },
        { EL_SV("while"),    EL_TT_KW_WHILE      },
        { EL_SV("extern"),   EL_TT_KW_EXTERN     },
        { EL_SV("return"),   EL_TT_KW_RETURN     },
        { EL_SV("static"),   EL_TT_KW_STATIC     },
        { EL_SV("struct"),   EL_TT_KW_STRUCT     },
        { EL_SV("switch"),   EL_TT_KW_SWITCH     },
        { EL_SV("bitcast"),  EL_TT_KW_BITCAST    },
        { EL_SV("default"),  EL_TT_KW_DEFAULT    },
        { EL_SV("typedef"),  EL_TT_KW_TYPEDEF    },
        { EL_SV("internal"), EL_TT_KW_INTERNAL   },
        { EL_SV("continue"), EL_TT_KW_CONTINUE   },
        { EL_SV("volatile"), EL_TT_KW_VOLATILE   },
    };

    static usize keywords_count = sizeof(keywords) / sizeof(keywords[0]);
    for (StringToKeyword* pair = keywords; pair < keywords+keywords_count; ++pair) {
        if (el_sv_eql(lexeme, pair->str)) {
            return pair->kwtype;
        }
    }

    return EL_TT_IDENT;
}


static ElLexerStatus lex_op2(ElLexer* lexer, char expect, ElTokenType single, ElTokenType dbl, ElToken* out) {
    if (peek(lexer) == expect) {
        next(lexer);
        return ret_tok_with_lexeme_auto(lexer, dbl, out);
    }

    return
        ret_tok_with_lexeme_auto(lexer, single, out);
}

static ElLexerStatus lex_op3(ElLexer* lexer, char expect1, char expect2, ElTokenType single, ElTokenType dbl, ElTokenType triple, ElToken* out) {
    if (peek(lexer) == expect1) {
        next(lexer);
        if (peek(lexer) == expect2) {
            next(lexer);
            return ret_tok_with_lexeme_auto(lexer, triple, out);
        }
        return ret_tok_with_lexeme_auto(lexer, dbl, out);
    }

    return ret_tok_with_lexeme_auto(lexer, single, out);
}

static ElLexerStatus lex_operator(ElLexer* lexer, char c, ElToken* out) {
    switch (c) {
    case '?':
        if (peek(lexer) == '?') {
            next(lexer);
            if (peek(lexer) == '=') {
                next(lexer);
                return ret_tok_with_lexeme_auto(lexer, EL_TT_OPT_FB_ASSIGN, out);
            }
            return ret_tok_with_lexeme_auto(lexer, EL_TT_OPT_FB, out);
        }
        if (peek(lexer) == '>') {
            next(lexer);
            return ret_tok_with_lexeme_auto(lexer, EL_TT_OPT_MAP, out);
        }
        if (peek(lexer) == '.') {
            next(lexer);
            return ret_tok_with_lexeme_auto(lexer, EL_TT_OPT_DOT, out);
        }
        return ret_tok_with_lexeme_auto(lexer, EL_TT_OPT, out);

    case '+':
        if (peek(lexer) == '+') {
            next(lexer);
            return ret_tok_with_lexeme_auto(lexer, EL_TT_INC, out);
        }
        if (peek(lexer) == '=') {
            next(lexer);
            return ret_tok_with_lexeme_auto(lexer, EL_TT_ADD_ASSIGN, out);
        }
        return ret_tok_with_lexeme_auto(lexer, EL_TT_PLUS, out);
    case '-':
        if (peek(lexer) == '-') {
            next(lexer);
            return ret_tok_with_lexeme_auto(lexer, EL_TT_DEC, out);
        }
        if (peek(lexer) == '=') {
            next(lexer);
            return ret_tok_with_lexeme_auto(lexer, EL_TT_SUB_ASSIGN, out);
        }
        return ret_tok_with_lexeme_auto(lexer, EL_TT_MINUS, out);

    case '*': return lex_op2(lexer, '=', EL_TT_STAR, EL_TT_MUL_ASSIGN, out);
    case '%': return lex_op2(lexer, '=', EL_TT_PERCENT, EL_TT_MOD_ASSIGN, out);
    case '=':
        if (peek(lexer) == '=') {
            next(lexer);
            return ret_tok_with_lexeme_auto(lexer, EL_TT_EQL, out);
        }
        if (peek(lexer) == '>') {
            next(lexer);
            return lex_op2(lexer, '=', EL_TT_LOGICAL_IMP, EL_TT_LOGICAL_IMP_ASSIGN, out);
        }
        return ret_tok_with_lexeme_auto(lexer, EL_TT_ASSIGN, out);
    case '!': return lex_op2(lexer, '=', EL_TT_LOGICAL_NOT, EL_TT_NEQ, out);

    case ':': return lex_op2(lexer, ':', EL_TT_COLON, EL_TT_DOUBLECOLON, out);

    case '&':
        if (peek(lexer) == '&') {
            next(lexer);
            return lex_op2(lexer, '=', EL_TT_LOGICAL_AND, EL_TT_LOGICAL_AND_ASSIGN, out);
        }
        return lex_op2(lexer, '=', EL_TT_BITWISE_AND, EL_TT_BITWISE_AND_ASSIGN, out);

    case '|':
        if (peek(lexer) == '|') {
            next(lexer);
            return lex_op2(lexer, '=', EL_TT_LOGICAL_OR, EL_TT_LOGICAL_OR_ASSIGN, out);
        }
        return lex_op2(lexer, '=', EL_TT_BITWISE_OR, EL_TT_BITWISE_OR_ASSIGN, out);

    case '<':
        if (peek(lexer) == '<') {
            next(lexer);
            return lex_op2(lexer, '=', EL_TT_SHL, EL_TT_SHL_ASSIGN, out);
        } else if (peek(lexer) == '>') {
            next(lexer);
            return lex_op2(lexer, '=', EL_TT_BITWISE_XOR, EL_TT_BITWISE_XOR_ASSIGN, out);
        }
        return lex_op2(lexer, '=', EL_TT_LT, EL_TT_LTE, out);

    case '>':
        if (peek(lexer) == '>') {
            next(lexer);
            return lex_op2(lexer, '=', EL_TT_SHR, EL_TT_SHR_ASSIGN, out);
        }
        return lex_op2(lexer, '=', EL_TT_GT, EL_TT_GTE, out);

    case '(': return ret_tok_with_lexeme_auto(lexer, EL_TT_LPAREN, out);
    case ')': return ret_tok_with_lexeme_auto(lexer, EL_TT_RPAREN, out);
    case '[': return ret_tok_with_lexeme_auto(lexer, EL_TT_LBRACKET, out);
    case ']': return ret_tok_with_lexeme_auto(lexer, EL_TT_RBRACKET, out);
    case '{': return ret_tok_with_lexeme_auto(lexer, EL_TT_LBRACE, out);
    case '}': return ret_tok_with_lexeme_auto(lexer, EL_TT_RBRACE, out);
    case ';': return ret_tok_with_lexeme_auto(lexer, EL_TT_SEMICOLON, out);
    case '^': return ret_tok_with_lexeme_auto(lexer, EL_TT_CARET, out);
    case '#': return ret_tok_with_lexeme_auto(lexer, EL_TT_HASH, out);
    case ',': return ret_tok_with_lexeme_auto(lexer, EL_TT_COMMA, out);

    case '.':
        if (peek(lexer) == '.' && peek_next(lexer) == '.') {
            next(lexer);
            next(lexer);
            return ret_tok_with_lexeme_auto(lexer, EL_TT_ELLIPSIS, out);
        }
        return ret_tok_with_lexeme_auto(lexer, EL_TT_DOT, out);

    case '~': return lex_op3(lexer, '>', '=', EL_TT_BITWISE_NOT, EL_TT_BITWISE_IMP, EL_TT_BITWISE_IMP_ASSIGN, out);

    default:
        EL_LEXER_RETURN_ERROR(
            lexer, EL_LEXERR_UNEXPECTED_CHAR,
            el_srcspan_make(lexer->doc, lexer->token_start_loc, lexer->current_loc),
            { .ch = c }
        );
    }
}

#define UTF8_MULTIBYTE_MARKER 0x80

static ElLexerStatus lex_ident(ElLexer* lexer, ElToken* out) {
    while (isalnum(peek(lexer)) || peek(lexer) == '_') next(lexer);

    if (lexer->flags & EL_LF_ALLOW_UTF8_IDENTS) {
        while ((unsigned char)peek(lexer) >= UTF8_MULTIBYTE_MARKER && !isspace(peek(lexer))) {
            next(lexer);
        }
    }

    ElStringView lexeme = el_make_lexeme_from_token_start(lexer);
    return ret_token_with_lexeme(
        lexer, get_keyword_or_ident_type(lexeme), lexeme, out
    );
}

static bool is_number_char(char c) {
    return isalnum((unsigned char)c) || c == '\'';
}

static ElLexerStatus lex_number(ElLexer* lexer, ElToken* out) {
    bool is_float = false;

    while (is_number_char(peek(lexer))) next(lexer);

    if (peek(lexer) == '.' && isdigit((unsigned char)peek_next(lexer))) {
        is_float = true;
        next(lexer);
        while (is_number_char(peek(lexer))) next(lexer);
    }

    if (peek(lexer) == 'e' || peek(lexer) == 'E') {
        is_float = true;
        next(lexer);
        if (peek(lexer) == '+' || peek(lexer) == '-') next(lexer);
        while (is_number_char(peek(lexer))) next(lexer);
    }

    return ret_tok_with_lexeme_auto(lexer, is_float ? EL_TT_FLOAT_LITERAL : EL_TT_INT_LITERAL, out);
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity): it's ok.
static ElLexerStatus lex_quoted_literal(ElLexer* lexer, char delimiter, ElLexerStatus unterminated_err, ElTokenType type, ElToken* out) {
    ElStringView content = el_srcdoc_content(lexer->doc);
    bool terminated = false;

    while (peek(lexer) != '\0' && peek(lexer) != '\n' && peek(lexer) != '\r') {
        if (peek(lexer) == '\\') {
            next(lexer);
            if (peek(lexer) == '\0') {
                EL_LEXER_RETURN_ERROR(
                    lexer, EL_LEXERR_INVALID_ESCAPE,
                    el_srcspan_make(lexer->doc, lexer->current_loc, lexer->current_loc), {}
                );
            }
            next(lexer);
        } else {
            if (next(lexer) == delimiter) {
                terminated = true;
                break;
            }
        }
    }

    if (!terminated && !(lexer->flags & EL_LF_ALLOW_UNTERM)) {
        EL_LEXER_RETURN_ERROR(lexer, unterminated_err, el_srcspan_make(lexer->doc, lexer->token_start_loc, lexer->current_loc), {});
    }

    ElStringView lex = {
        .data = content.data + lexer->token_start_loc.offset + 1,
        .len = lexer->current_loc.offset - lexer->token_start_loc.offset - 2
    };

    return ret_token_with_lexeme(lexer, type, lex, out);
}

// TODO: refactor this function
// NOLINTNEXTLINE
ElLexerStatus el_lexer_next_token(ElLexer* lexer, ElToken* out) {
    ElStringView content = el_srcdoc_content(lexer->doc);
    while (true) {
        lexer->token_start_loc = lexer->current_loc;

        char c = peek(lexer);
        if (c == '\0') {
            if (lexer->current_loc.offset < content.len) {
                next(lexer);
                EL_LEXER_RETURN_ERROR(lexer, EL_LEXERR_UNEXPECTED_CHAR, el_srcspan_make(lexer->doc, lexer->token_start_loc, lexer->current_loc), {});
            }
            return ret_token(lexer, EL_TT_EOF, out);
        }

        if (isspace(c)) {
            if (c == '\n' || c == '\r') {
                next(lexer);

                if (lexer->flags & EL_LF_SKIP_WHITESPACE) continue;
                return ret_tok_with_lexeme_auto(lexer, EL_TT_NEWLINE, out);
            }

            if (lexer->flags & EL_LF_SKIP_WHITESPACE) {
                while (isspace(peek(lexer)) && peek(lexer) != '\n' && peek(lexer) != '\r' && peek(lexer) != '\0')
                    next(lexer);
                continue;
            }

            next(lexer);
            while (isspace(peek(lexer)) && peek(lexer) != '\n' && peek(lexer) != '\r' && peek(lexer) != '\0') next(lexer);

            return ret_tok_with_lexeme_auto(lexer, EL_TT_WHITESPACE, out);
        }

        if (c == '/') {
            next(lexer);
            if (peek(lexer) == '/') {
                next(lexer);
                usize content_start_offset = lexer->current_loc.offset;

                while (peek(lexer) != '\n' && peek(lexer) != '\r' && peek(lexer) != '\0') next(lexer);

                if (lexer->flags & EL_LF_SKIP_COMMENTS) continue;

                ElStringView lexeme = {
                    .data = content.data + content_start_offset,
                    .len = lexer->current_loc.offset - content_start_offset
                };
                return ret_token_with_lexeme(lexer, EL_TT_LINE_COMMENT, lexeme, out);
            }
            if (peek(lexer) == '*') {
                next(lexer);
                usize content_start_offset = lexer->current_loc.offset;

                bool terminated = false;
                usize content_end_offset = lexer->current_loc.offset;

                while (peek(lexer) != '\0') {
                    if (peek(lexer) == '*' && peek_next(lexer) == '/') {
                        content_end_offset = lexer->current_loc.offset;
                        next(lexer);
                        next(lexer);
                        terminated = true;
                        break;
                    }
                    next(lexer);
                }

                if (lexer->flags & EL_LF_SKIP_COMMENTS) {
                    if (!terminated && !(lexer->flags & EL_LF_ALLOW_UNTERM)) {
                        EL_LEXER_RETURN_ERROR(lexer, EL_LEXERR_UNTERM_COMMENT, el_srcspan_make(lexer->doc, lexer->token_start_loc, lexer->current_loc), {});
                    }
                    continue;
                }

                if (!terminated) {
                    if (!(lexer->flags & EL_LF_ALLOW_UNTERM)) {
                        EL_LEXER_RETURN_ERROR(lexer, EL_LEXERR_UNTERM_COMMENT, el_srcspan_make(lexer->doc, lexer->token_start_loc, lexer->current_loc), {});
                    }
                    ElStringView lexeme = {
                        .data = content.data + content_start_offset,
                        .len = lexer->current_loc.offset - content_start_offset
                    };
                    return ret_token_with_lexeme(lexer, EL_TT_BLOCK_COMMENT, lexeme, out);
                }

                ElStringView lexeme = {
                    .data = content.data + content_start_offset,
                    .len = content_end_offset - content_start_offset
                };
                return ret_token_with_lexeme(lexer, EL_TT_BLOCK_COMMENT, lexeme, out);
            }
            if (peek(lexer) == '=') {
                next(lexer);
                return ret_tok_with_lexeme_auto(lexer, EL_TT_DIV_ASSIGN, out);
            }
            return ret_tok_with_lexeme_auto(lexer, EL_TT_SLASH, out);
        }

        if (c == '"') {
            next(lexer);
            return lex_quoted_literal(lexer, '"', EL_LEXERR_UNTERM_STRING, EL_TT_STRING_LITERAL, out);
        }

        if (c == '\'') {
            next(lexer);
            return lex_quoted_literal(lexer, '\'', EL_LEXERR_UNTERM_CHAR, EL_TT_CHAR_LITERAL, out);
        }

        if (isalpha(c) || c == '_') {
            next(lexer);
            return lex_ident(lexer, out);
        }

        if (isdigit(c)) {
            next(lexer);
            return lex_number(lexer, out);
        }

        char op = next(lexer);

        ElLexerStatus r = lex_operator(lexer, op, out);
        if (r != EL_LEXERR_UNEXPECTED_CHAR) return r;

        if (!(lexer->flags & EL_LF_SKIP_UNKNOWN)) {
            EL_LEXER_RETURN_ERROR(lexer, EL_LEXERR_UNEXPECTED_CHAR, el_srcspan_make(lexer->doc, lexer->token_start_loc, lexer->current_loc), {});
        }
    }
}

static ElToken token_stream_next(ElTokenStream* stream, ElDiagEngine* engine) {
    ElLexer* lexer = (ElLexer*)stream->ctx;
    ElProfScope* scope = el_prof_enter_stage(lexer->prof, lexer->prof_stage);

    ElToken tok;
    ElLexerStatus err = el_lexer_next_token(lexer, &tok);

    if (err != EL_LEXERR_SUCCESS) {
        if (engine != NULL) {
            el_diag_handle_lexer_error(engine, &lexer->last_err_details);
        }

        tok.type = EL_TT_UNKNOWN;
        tok.span = lexer->last_err_details.span;
        tok.lexeme = el_srcspan_to_sv(tok.span);
    }

    el_prof_leave_stage(lexer->prof, scope);

    return tok;
}

ElTokenStream el_lexer_as_token_stream(ElLexer* lexer) {
    return (ElTokenStream) {
        .next = token_stream_next,
        .ctx = lexer,
        .prof = lexer->prof,
        .prof_stage = lexer->prof_stage,
    };
}
