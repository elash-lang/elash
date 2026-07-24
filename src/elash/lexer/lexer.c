#include <elash/lexer/lexer.h>
#include <elash/lexer/token.h>
#include <elash/lexer/macros.h>

#include <elash/srcdoc/srcdoc.h>
#include <elash/diag/handle.h>
#include <elash/defs/sv.h>

#include <ctype.h>
#include <stdbool.h>
#include <string.h>

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
        if (peek(lexer) == '\n') { // CRLF
            lexer->current_loc.offset++;
        }

        lexer->current_loc.line++;
        lexer->current_loc.column = 0;
    } else if (c == '\n') {
        lexer->current_loc.line++;
        lexer->current_loc.column = 0;
    }

    return c;
}

static inline ElStringView el_make_lexeme_from_token_start(ElLexer* lexer) {
    return
        el_sv_slice(el_srcdoc_content(lexer->doc), lexer->token_start_loc.offset, lexer->current_loc.offset);
}

ElLexerErrorCode _el_lexer_ret_token(ElLexer* lexer, ElTokenType type, ElToken* out_tok) {
    lexer->last_err_details = EL_LEXER_RESULT_SUCCESS;

    out_tok->type = type;
    out_tok->span = el_source_span_make(lexer->doc, lexer->token_start_loc, lexer->current_loc);
    out_tok->lexeme = el_source_span_to_sv(out_tok->span);

    return EL_LEXERR_SUCCESS;
}

ElLexerErrorCode _el_lexer_ret_token_with_lexeme(ElLexer* lexer, ElTokenType type, ElStringView lexeme, ElToken* out_tok) {
    lexer->last_err_details = EL_LEXER_RESULT_SUCCESS;

    out_tok->type = type;
    out_tok->span = el_source_span_make(lexer->doc, lexer->token_start_loc, lexer->current_loc);
    out_tok->lexeme = lexeme;

    return EL_LEXERR_SUCCESS;
}

ElLexerErrorCode el_lexer_init(ElLexer* lexer, const ElSourceDocument* doc, ElLexerFlags flags) {
    lexer->doc = doc;
    lexer->current_loc = EL_SOURCE_LOC_ZERO;
    lexer->token_start_loc = EL_SOURCE_LOC_ZERO;
    lexer->last_err_details = EL_LEXER_RESULT_SUCCESS;
    lexer->flags = flags;
    lexer->ctx = EL_LEXER_CTX_DEFAULT;
    lexer->prev_ctx = EL_LEXER_CTX_DEFAULT;

    return EL_LEXERR_SUCCESS;
}

ElLexerErrorCode el_lexer_reset(ElLexer* lexer) {
    lexer->current_loc = EL_SOURCE_LOC_ZERO;
    lexer->token_start_loc = EL_SOURCE_LOC_ZERO;
    lexer->doc = NULL;
    lexer->last_err_details.code = EL_LEXERR_SUCCESS;
    lexer->ctx = EL_LEXER_CTX_DEFAULT;
    lexer->prev_ctx = EL_LEXER_CTX_DEFAULT;

    return EL_LEXERR_SUCCESS;
}

ElLexerErrorCode el_lexer_set_document(ElLexer* lexer, const ElSourceDocument* doc) {
    ElLexerErrorCode err = el_lexer_reset(lexer);

    lexer->doc = doc;

    return err;
}

ElTokenType _el_lexer_get_keyword_or_ident_type(ElStringView lexeme, ElLexerContext ctx) {
    typedef struct StringToKeyword {
        ElStringView str;
        ElTokenType kwtype;
    } StringToKeyword;

    static StringToKeyword default_keywords[] = {
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
        { EL_SV("write"),    EL_TT_KW_WRITE      },
        { EL_SV("alias"),    EL_TT_KW_ALIAS      },
        { EL_SV("false"),    EL_TT_FALSE_LITERAL },
        { EL_SV("union"),    EL_TT_KW_UNION      },
        { EL_SV("while"),    EL_TT_KW_WHILE      },
        { EL_SV("extern"),   EL_TT_KW_EXTERN     },
        { EL_SV("inline"),   EL_TT_KW_INLINE     },
        { EL_SV("return"),   EL_TT_KW_RETURN     },
        { EL_SV("static"),   EL_TT_KW_STATIC     },
        { EL_SV("struct"),   EL_TT_KW_STRUCT     },
        { EL_SV("switch"),   EL_TT_KW_SWITCH     },
        { EL_SV("sizeof"),   EL_TT_SIZEOF        },
        { EL_SV("default"),  EL_TT_KW_DEFAULT    },
        { EL_SV("typedef"),  EL_TT_KW_TYPEDEF    },
        { EL_SV("continue"), EL_TT_KW_CONTINUE   },
        { EL_SV("volatile"), EL_TT_KW_VOLATILE   },
    };
    static usize default_keywords_size = sizeof(default_keywords) / sizeof(default_keywords[0]);

    static StringToKeyword pp_keywords[] = {
        { EL_SV("if"),       EL_TT_PP_IF       },
        { EL_SV("for"),      EL_TT_PP_FOR      },
        { EL_SV("end"),      EL_TT_PP_END      },
        { EL_SV("dec"),      EL_TT_PP_DEC      },
        { EL_SV("inc"),      EL_TT_PP_INC      },
        { EL_SV("elif"),     EL_TT_PP_ELSE     },
        { EL_SV("else"),     EL_TT_PP_ELSE     },
        { EL_SV("note"),     EL_TT_PP_NOTE     },
        { EL_SV("emit"),     EL_TT_PP_EMIT     },
        { EL_SV("embed"),    EL_TT_PP_EMBED    },
        { EL_SV("undef"),    EL_TT_PP_UNDEF    },
        { EL_SV("while"),    EL_TT_PP_WHILE    },
        { EL_SV("debug"),    EL_TT_PP_DEBUG    },
        { EL_SV("assign"),   EL_TT_PP_ASSIGN   },
        { EL_SV("pragma"),   EL_TT_PP_PRAGMA   },
        { EL_SV("define"),   EL_TT_PP_DEFINE   },
        { EL_SV("include"),  EL_TT_PP_INCLUDE  },
        { EL_SV("foreach"),  EL_TT_PP_FOREACH  },
    };
    static usize pp_keywords_size = sizeof(pp_keywords) / sizeof(pp_keywords[0]);

    if (ctx == EL_LEXER_CTX_PP) {
        for (StringToKeyword* pair = pp_keywords; pair < pp_keywords+pp_keywords_size; ++pair) {
            if (el_sv_eql(lexeme, pair->str)) {
                return pair->kwtype;
            }
        }
    }
    for (StringToKeyword* pair = default_keywords; pair < default_keywords+default_keywords_size; ++pair) {
        if (el_sv_eql(lexeme, pair->str)) {
            return pair->kwtype;
        }
    }

    return EL_TT_IDENT;
}

static inline ElLexerErrorCode _el_lexer_ret_tok_with_lexeme_auto(ElLexer* lexer, ElTokenType t, ElToken* out) {
    return
        _el_lexer_ret_token_with_lexeme(lexer, t, el_make_lexeme_from_token_start(lexer), out);
}

ElLexerErrorCode _el_lexer_lex_op2(ElLexer* lexer, char expect, ElTokenType single, ElTokenType dbl, ElToken* out) {
    if (peek(lexer) == expect) {
        next(lexer);
        return _el_lexer_ret_tok_with_lexeme_auto(lexer, dbl, out);
    }

    return
        _el_lexer_ret_tok_with_lexeme_auto(lexer, single, out);
}

ElLexerErrorCode _el_lexer_lex_op3(ElLexer* lexer, char expect1, char expect2, ElTokenType single, ElTokenType dbl, ElTokenType triple, ElToken* out) {
    if (peek(lexer) == expect1) {
        next(lexer);
        if (peek(lexer) == expect2) {
            next(lexer);
            return _el_lexer_ret_tok_with_lexeme_auto(lexer, triple, out);
        }
        return _el_lexer_ret_tok_with_lexeme_auto(lexer, dbl, out);
    }

    return _el_lexer_ret_tok_with_lexeme_auto(lexer, single, out);
}

static ElLexerErrorCode lex_operator(ElLexer* lexer, char c, ElToken* out) {
    switch (c) {
    case '+':
        if (peek(lexer) == '+') {
            next(lexer);
            return _el_lexer_ret_tok_with_lexeme_auto(lexer, EL_TT_INC, out);
        }
        if (peek(lexer) == '=') {
            next(lexer);
            return _el_lexer_ret_tok_with_lexeme_auto(lexer, EL_TT_ADD_ASSIGN, out);
        }
        return _el_lexer_ret_tok_with_lexeme_auto(lexer, EL_TT_PLUS, out);
    case '-':
        if (peek(lexer) == '-') {
            next(lexer);
            return _el_lexer_ret_tok_with_lexeme_auto(lexer, EL_TT_DEC, out);
        }
        if (peek(lexer) == '=') {
            next(lexer);
            return _el_lexer_ret_tok_with_lexeme_auto(lexer, EL_TT_SUB_ASSIGN, out);
        }
        return _el_lexer_ret_tok_with_lexeme_auto(lexer, EL_TT_MINUS, out);

    case '*': return _el_lexer_lex_op2(lexer, '=', EL_TT_STAR, EL_TT_MUL_ASSIGN, out);
    case '%': return _el_lexer_lex_op2(lexer, '=', EL_TT_PERCENT, EL_TT_MOD_ASSIGN, out);
    case '=':
        if (peek(lexer) == '=') {
            next(lexer);
            return _el_lexer_ret_tok_with_lexeme_auto(lexer, EL_TT_EQL, out);
        }
        if (peek(lexer) == '>') {
            next(lexer);
            return _el_lexer_lex_op2(lexer, '=', EL_TT_LOGICAL_IMP, EL_TT_LOGICAL_IMP_ASSIGN, out);
        }
        return _el_lexer_ret_tok_with_lexeme_auto(lexer, EL_TT_ASSIGN, out);
    case '!': return _el_lexer_lex_op2(lexer, '=', EL_TT_LOGICAL_NOT, EL_TT_NEQ, out);

    case ':': return _el_lexer_lex_op2(lexer, ':', EL_TT_COLON, EL_TT_DOUBLECOLON, out);

    case '&':
        if (peek(lexer) == '&') {
            next(lexer);
            return _el_lexer_lex_op2(lexer, '=', EL_TT_LOGICAL_AND, EL_TT_LOGICAL_AND_ASSIGN, out);
        }
        return _el_lexer_lex_op2(lexer, '=', EL_TT_BITWISE_AND, EL_TT_BITWISE_AND_ASSIGN, out);

    case '|':
        if (peek(lexer) == '|') {
            next(lexer);
            return _el_lexer_lex_op2(lexer, '=', EL_TT_LOGICAL_OR, EL_TT_LOGICAL_OR_ASSIGN, out);
        }
        return _el_lexer_lex_op2(lexer, '=', EL_TT_BITWISE_OR, EL_TT_BITWISE_OR_ASSIGN, out);

    case '<':
        if (peek(lexer) == '<') {
            next(lexer);
            return _el_lexer_lex_op2(lexer, '=', EL_TT_SHL, EL_TT_SHL_ASSIGN, out);
        } else if (peek(lexer) == '>') {
            next(lexer);
            return _el_lexer_lex_op2(lexer, '=', EL_TT_BITWISE_XOR, EL_TT_BITWISE_XOR_ASSIGN, out);
        }
        return _el_lexer_lex_op2(lexer, '=', EL_TT_LT, EL_TT_LTE, out);

    case '>':
        if (peek(lexer) == '>') {
            next(lexer);
            return _el_lexer_lex_op2(lexer, '=', EL_TT_SHR, EL_TT_SHR_ASSIGN, out);
        }
        return _el_lexer_lex_op2(lexer, '=', EL_TT_GT, EL_TT_GTE, out);

    case '(': return _el_lexer_ret_tok_with_lexeme_auto(lexer, EL_TT_LPAREN, out);
    case ')': return _el_lexer_ret_tok_with_lexeme_auto(lexer, EL_TT_RPAREN, out);
    case '[': return _el_lexer_ret_tok_with_lexeme_auto(lexer, EL_TT_LBRACKET, out);
    case ']': return _el_lexer_ret_tok_with_lexeme_auto(lexer, EL_TT_RBRACKET, out);
    case '{': return _el_lexer_ret_tok_with_lexeme_auto(lexer, EL_TT_LBRACE, out);
    case '}': return _el_lexer_ret_tok_with_lexeme_auto(lexer, EL_TT_RBRACE, out);
    case ';': return _el_lexer_ret_tok_with_lexeme_auto(lexer, EL_TT_SEMICOLON, out);
    case '^': return _el_lexer_ret_tok_with_lexeme_auto(lexer, EL_TT_CARET, out);
    case '.':
        if (peek(lexer) == '.' && peek_next(lexer) == '.') {
            next(lexer);
            next(lexer);
            return _el_lexer_ret_tok_with_lexeme_auto(lexer, EL_TT_ELLIPSIS, out);
        }
        return _el_lexer_ret_tok_with_lexeme_auto(lexer, EL_TT_DOT, out);
    case ',': return _el_lexer_ret_tok_with_lexeme_auto(lexer, EL_TT_COMMA, out);
    case '~': return _el_lexer_lex_op3(lexer, '>', '=', EL_TT_BITWISE_NOT, EL_TT_BITWISE_IMP, EL_TT_BITWISE_IMP_ASSIGN, out);

    case '#':
        lexer->ctx = EL_LEXER_CTX_PP;
        return _el_lexer_ret_tok_with_lexeme_auto(lexer, EL_TT_HASH, out);

    default:
        return EL_LEXERR_UNEXPECTED_CHAR;
    }
}

#define UTF8_MULTIBYTE_MARKER 0x80

static ElLexerErrorCode lex_ident(ElLexer* lexer, ElLexerContext prev_ctx, ElToken* out) {
    while (isalnum(peek(lexer)) || peek(lexer) == '_') next(lexer);

    if (lexer->flags & EL_LF_ALLOW_UTF8_IDENTS) {
        while ((unsigned char)peek(lexer) >= UTF8_MULTIBYTE_MARKER && !isspace(peek(lexer))) {
            next(lexer);
        }
    }

    ElStringView lex = el_make_lexeme_from_token_start(lexer);

    return _el_lexer_ret_token_with_lexeme(lexer, _el_lexer_get_keyword_or_ident_type(lex, prev_ctx), lex, out);
}

static ElLexerErrorCode lex_number(ElLexer* lexer, ElToken* out) {
    bool is_float = false;

    while (isdigit(peek(lexer))) next(lexer);

    if (peek(lexer) == '.') {
        is_float = true;
        next(lexer);
        while (isdigit(peek(lexer))) next(lexer);
    }

    if (peek(lexer) == 'e' || peek(lexer) == 'E') {
        is_float = true;
        next(lexer);
        if (peek(lexer) == '+' || peek(lexer) == '-') next(lexer);
        while (isdigit(peek(lexer))) next(lexer);
    }

    return _el_lexer_ret_tok_with_lexeme_auto(lexer, is_float ? EL_TT_FLOAT_LITERAL : EL_TT_INT_LITERAL, out);
}

// TODO: refactor this function
// NOLINTNEXTLINE
ElLexerErrorCode el_lexer_next_token(ElLexer* lexer, ElToken* out) {
    ElStringView content = el_srcdoc_content(lexer->doc);

    lexer->prev_ctx = lexer->ctx;
    while (true) {
        lexer->token_start_loc = lexer->current_loc;

        char c = peek(lexer);
        if (c == '\0') {
            if (lexer->current_loc.offset < content.len) {
                next(lexer);
                EL_LEXER_RETURN_ERROR(lexer, EL_LEXERR_UNEXPECTED_CHAR, el_source_span_make(lexer->doc, lexer->token_start_loc, lexer->current_loc), {});
            }
            return _el_lexer_ret_token(lexer, EL_TT_EOF, out);
        }

        if (isspace(c)) {
            if (c == '\n' || c == '\r') {
                next(lexer);

                if (lexer->ctx == EL_LEXER_CTX_PP) {
                    lexer->ctx = EL_LEXER_CTX_DEFAULT;
                }

                if (lexer->flags & EL_LF_SKIP_WHITESPACE) {
                    lexer->ctx = EL_LEXER_CTX_DEFAULT;
                    continue;
                }

                return _el_lexer_ret_tok_with_lexeme_auto(lexer, EL_TT_NEWLINE, out);
            }

            if (lexer->flags & EL_LF_SKIP_WHITESPACE) {
                while (isspace(peek(lexer)) && peek(lexer) != '\n' && peek(lexer) != '\r' && peek(lexer) != '\0')
                    next(lexer);
                continue;
            }

            next(lexer);
            while (isspace(peek(lexer)) && peek(lexer) != '\n' && peek(lexer) != '\r' && peek(lexer) != '\0') next(lexer);

            return _el_lexer_ret_tok_with_lexeme_auto(lexer, EL_TT_WHITESPACE, out);
        }

        if (c == '/') {
            next(lexer);
            if (peek(lexer) == '/') {
                next(lexer);
                usize content_start_offset = lexer->current_loc.offset;

                while (peek(lexer) != '\n' && peek(lexer) != '\0') next(lexer);

                if (lexer->flags & EL_LF_SKIP_COMMENTS) continue;

                ElStringView lexeme = {
                    .data = content.data + content_start_offset,
                    .len = lexer->current_loc.offset - content_start_offset
                };
                return _el_lexer_ret_token_with_lexeme(lexer, EL_TT_LINE_COMMENT, lexeme, out);
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
                        EL_LEXER_RETURN_ERROR(lexer, EL_LEXERR_UNTERM_COMMENT, el_source_span_make(lexer->doc, lexer->token_start_loc, lexer->current_loc), {});
                    }
                    continue;
                }

                if (!terminated) {
                    if (!(lexer->flags & EL_LF_ALLOW_UNTERM)) {
                        EL_LEXER_RETURN_ERROR(lexer, EL_LEXERR_UNTERM_COMMENT, el_source_span_make(lexer->doc, lexer->token_start_loc, lexer->current_loc), {});
                    }
                    ElStringView lexeme = {
                        .data = content.data + content_start_offset,
                        .len = lexer->current_loc.offset - content_start_offset
                    };
                    return _el_lexer_ret_token_with_lexeme(lexer, EL_TT_BLOCK_COMMENT, lexeme, out);
                }

                ElStringView lexeme = {
                    .data = content.data + content_start_offset,
                    .len = content_end_offset - content_start_offset
                };
                return _el_lexer_ret_token_with_lexeme(lexer, EL_TT_BLOCK_COMMENT, lexeme, out);
            }
            if (peek(lexer) == '=') {
                next(lexer);
                return _el_lexer_ret_tok_with_lexeme_auto(lexer, EL_TT_DIV_ASSIGN, out);
            }
            return _el_lexer_ret_tok_with_lexeme_auto(lexer, EL_TT_SLASH, out);
        }

        if (c == '"') {
            next(lexer);

            bool terminated = false;

            while (peek(lexer) != '\0' && peek(lexer) != '\n') {
                if (peek(lexer) == '\\') {
                    next(lexer);
                    if (peek(lexer) == '\0') EL_LEXER_RETURN_ERROR(lexer, EL_LEXERR_INVALID_ESCAPE, el_source_span_make(lexer->doc, lexer->current_loc, lexer->current_loc), {});
                    next(lexer);
                } else {
                    if (next(lexer) == '"') {
                        terminated = true;
                        break;
                    }
                }
            }

            if (!terminated && !(lexer->flags & EL_LF_ALLOW_UNTERM)) EL_LEXER_RETURN_ERROR(lexer, EL_LEXERR_UNTERM_STRING, el_source_span_make(lexer->doc, lexer->token_start_loc, lexer->current_loc), {});

            ElStringView lex = {
                .data = content.data + lexer->token_start_loc.offset + 1,
                .len = lexer->current_loc.offset - lexer->token_start_loc.offset - 2
            };

            return _el_lexer_ret_token_with_lexeme(lexer, EL_TT_STRING_LITERAL, lex, out);
        }

        if (c == '\'') {
            next(lexer);

            if (peek(lexer) == '\\') {
                next(lexer);
                next(lexer);
            } else {
                next(lexer);
            }

            if (peek(lexer) != '\'') EL_LEXER_RETURN_ERROR(lexer, EL_LEXERR_UNTERM_CHAR, el_source_span_make(lexer->doc, lexer->token_start_loc, lexer->current_loc), {});

            next(lexer);

            ElStringView lex = {
                .data = content.data + lexer->token_start_loc.offset + 1,
                .len = lexer->current_loc.offset - lexer->token_start_loc.offset - 2
            };
            return _el_lexer_ret_token_with_lexeme(lexer, EL_TT_CHAR_LITERAL, lex, out);
        }

        ElLexerContext prev_ctx = lexer->ctx;

        if (isalpha(c) || c == '_') {
            next(lexer);
            return lex_ident(lexer, prev_ctx, out);
        }

        if (isdigit(c)) {
            next(lexer);
            return lex_number(lexer, out);
        }

        char op = next(lexer);
        bool ctx_before_op_lex = lexer->ctx;
        ElLexerErrorCode r = lex_operator(lexer, op, out);

        if (r == EL_LEXERR_SUCCESS && out->type == EL_TT_SEMICOLON && ctx_before_op_lex) {
            // end pp context on semicolon
            lexer->ctx = EL_LEXER_CTX_DEFAULT;
        }

        if (r != EL_LEXERR_UNEXPECTED_CHAR) return r;

        if (!(lexer->flags & EL_LF_SKIP_UNKNOWN)) EL_LEXER_RETURN_ERROR(lexer, EL_LEXERR_UNEXPECTED_CHAR, el_source_span_make(lexer->doc, lexer->token_start_loc, lexer->current_loc), {});
    }
}

ElLexerContext el_lexer_get_current_context(const ElLexer* lexer) {
    return lexer->ctx;
}

ElLexerContext el_lexer_get_previous_context(const ElLexer* lexer) {
    return lexer->prev_ctx;
}

bool el_lexer_entered_context(const ElLexer* lexer, ElLexerContext context) {
    return lexer->ctx == context && lexer->prev_ctx != context;
}

bool el_lexer_exited_context(const ElLexer* lexer, ElLexerContext context) {
    return lexer->ctx != context && lexer->prev_ctx == context;
}

static ElToken _el_lexer_token_stream_next(ElTokenStream* stream, ElDiagEngine* engine) {
    ElLexer* lexer = (ElLexer*)stream->ctx;
    ElToken tok;
    ElLexerErrorCode err = el_lexer_next_token(lexer, &tok);

    if (err != EL_LEXERR_SUCCESS) {
        if (engine) {
            el_diag_handle_lexer_error(engine, &lexer->last_err_details);
        }

        tok.type = EL_TT_UNKNOWN;
        tok.span = lexer->last_err_details.span;
        tok.lexeme = el_source_span_to_sv(tok.span);
    }

    return tok;
}

ElTokenStream el_lexer_as_token_stream(ElLexer* lexer) {
    return (ElTokenStream) {
        .next = _el_lexer_token_stream_next,
        .ctx = lexer,
    };
}
