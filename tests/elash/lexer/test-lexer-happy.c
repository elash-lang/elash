#include <criterion/criterion.h>

#include <elash/lexer/lexer.h>
#include <elash/srcdoc/srcdoc.h>

// NOLINTNEXTLINE(readability-function-cognitive-complexity): clang-tidy sucks.
static void assert_token(ElLexer* lexer, ElTokenType expected_type, const char* expected_lexeme) {
    ElToken tok;
    ElLexerErrorCode err = el_lexer_next_token(lexer, &tok);

    ElStringView err_str = el_lexer_err_code_to_string(err);
    cr_assert_eq(err, EL_LEXERR_SUCCESS,
                 "Expected EL_LEXERR_SUCCESS, got " EL_SV_FMT, EL_SV_FARG(err_str));

    ElStringView actual_type_str = el_token_type_to_string(tok.type);
    ElStringView expected_type_str = el_token_type_to_string(expected_type);
    cr_assert_eq(tok.type, expected_type,
                 "Expected token type " EL_SV_FMT ", got " EL_SV_FMT " (lexeme: " EL_SV_FMT ")",
                 EL_SV_FARG(expected_type_str), EL_SV_FARG(actual_type_str), EL_SV_FARG(tok.lexeme));

    if (expected_lexeme) {
        ElStringView expected_sv = el_sv_from_cstr(expected_lexeme);
        cr_assert(el_sv_eql(tok.lexeme, expected_sv),
                  "Expected lexeme '" EL_SV_FMT "', got '" EL_SV_FMT "'",
                  EL_SV_FARG(expected_sv), EL_SV_FARG(tok.lexeme));
    }
}

Test(elash_lexer_happy, basic_tokens) {
    ElSourceDocument doc;
    el_srcdoc_init_from_str(&doc, EL_SV("foo bar 123 3.14 'a' \"hello\""), EL_SV("test.ei"));

    ElLexer lexer;
    el_lexer_init(&lexer, &doc, EL_LEXER_FLAGS_DEFAULT);

    assert_token(&lexer, EL_TT_IDENT, "foo");
    assert_token(&lexer, EL_TT_IDENT, "bar");
    assert_token(&lexer, EL_TT_INT_LITERAL, "123");
    assert_token(&lexer, EL_TT_FLOAT_LITERAL, "3.14");
    assert_token(&lexer, EL_TT_CHAR_LITERAL, "a");
    assert_token(&lexer, EL_TT_STRING_LITERAL, "hello");
    assert_token(&lexer, EL_TT_EOF, "");

    el_srcdoc_destroy(&doc);
}

Test(elash_lexer_happy, keywords) {
    ElSourceDocument doc;
    el_srcdoc_init_from_str(&doc, EL_SV("if else while for return break continue"), EL_SV("test.ei"));

    ElLexer lexer;
    el_lexer_init(&lexer, &doc, EL_LEXER_FLAGS_DEFAULT);

    assert_token(&lexer, EL_TT_KW_IF, "if");
    assert_token(&lexer, EL_TT_KW_ELSE, "else");
    assert_token(&lexer, EL_TT_KW_WHILE, "while");
    assert_token(&lexer, EL_TT_KW_FOR, "for");
    assert_token(&lexer, EL_TT_KW_RETURN, "return");
    assert_token(&lexer, EL_TT_KW_BREAK, "break");
    assert_token(&lexer, EL_TT_KW_CONTINUE, "continue");
    assert_token(&lexer, EL_TT_EOF, "");

    el_srcdoc_destroy(&doc);
}

Test(elash_lexer_happy, operators) {
    ElSourceDocument doc;
    el_srcdoc_init_from_str(&doc,
        EL_SV(
            "+ - * / % ++ -- += -= *= /= %="
            "= == != < <= > >="
            "&& || ! => &&= ||= =>="
            "& | ^ ~ ~> &= |= <> <>= ~>="
            "<< >> <<= >>="
        ), EL_SV("test.ei")
    );

    ElLexer lexer;
    el_lexer_init(&lexer, &doc, EL_LEXER_FLAGS_DEFAULT);

    assert_token(&lexer, EL_TT_PLUS, "+");
    assert_token(&lexer, EL_TT_MINUS, "-");
    assert_token(&lexer, EL_TT_STAR, "*");
    assert_token(&lexer, EL_TT_SLASH, "/");
    assert_token(&lexer, EL_TT_PERCENT, "%");
    assert_token(&lexer, EL_TT_INC, "++");
    assert_token(&lexer, EL_TT_DEC, "--");

    assert_token(&lexer, EL_TT_ADD_ASSIGN, "+=");
    assert_token(&lexer, EL_TT_SUB_ASSIGN, "-=");
    assert_token(&lexer, EL_TT_MUL_ASSIGN, "*=");
    assert_token(&lexer, EL_TT_DIV_ASSIGN, "/=");
    assert_token(&lexer, EL_TT_MOD_ASSIGN, "%=");

    assert_token(&lexer, EL_TT_ASSIGN, "=");
    assert_token(&lexer, EL_TT_EQL, "==");
    assert_token(&lexer, EL_TT_NEQ, "!=");
    assert_token(&lexer, EL_TT_LT, "<");
    assert_token(&lexer, EL_TT_LTE, "<=");
    assert_token(&lexer, EL_TT_GT, ">");
    assert_token(&lexer, EL_TT_GTE, ">=");

    assert_token(&lexer, EL_TT_LOGICAL_AND, "&&");
    assert_token(&lexer, EL_TT_LOGICAL_OR, "||");
    assert_token(&lexer, EL_TT_LOGICAL_NOT, "!");
    assert_token(&lexer, EL_TT_LOGICAL_IMP, "=>");
    assert_token(&lexer, EL_TT_LOGICAL_AND_ASSIGN, "&&=");
    assert_token(&lexer, EL_TT_LOGICAL_OR_ASSIGN,  "||=");
    assert_token(&lexer, EL_TT_LOGICAL_IMP_ASSIGN, "=>=");

    assert_token(&lexer, EL_TT_BITWISE_AND, "&");
    assert_token(&lexer, EL_TT_BITWISE_OR, "|");
    assert_token(&lexer, EL_TT_CARET, "^");
    assert_token(&lexer, EL_TT_BITWISE_NOT, "~");
    assert_token(&lexer, EL_TT_BITWISE_IMP, "~>");
    assert_token(&lexer, EL_TT_BITWISE_AND_ASSIGN, "&=");
    assert_token(&lexer, EL_TT_BITWISE_OR_ASSIGN, "|=");
    assert_token(&lexer, EL_TT_BITWISE_XOR, "<>");
    assert_token(&lexer, EL_TT_BITWISE_XOR_ASSIGN, "<>=");
    assert_token(&lexer, EL_TT_BITWISE_IMP_ASSIGN, "~>=");

    assert_token(&lexer, EL_TT_SHL, "<<");
    assert_token(&lexer, EL_TT_SHR, ">>");
    assert_token(&lexer, EL_TT_SHL_ASSIGN, "<<=");
    assert_token(&lexer, EL_TT_SHR_ASSIGN, ">>=");

    assert_token(&lexer, EL_TT_EOF, "");
    el_srcdoc_destroy(&doc);
}

Test(elash_lexer_happy, delimiters) {
    ElSourceDocument doc;
    el_srcdoc_init_from_str(&doc, EL_SV("( ) [ ] { } ; : :: , . # ..."), EL_SV("test.ei"));

    ElLexer lexer;
    el_lexer_init(&lexer, &doc, EL_LEXER_FLAGS_DEFAULT);

    assert_token(&lexer, EL_TT_LPAREN, "(");
    assert_token(&lexer, EL_TT_RPAREN, ")");
    assert_token(&lexer, EL_TT_LBRACKET, "[");
    assert_token(&lexer, EL_TT_RBRACKET, "]");
    assert_token(&lexer, EL_TT_LBRACE, "{");
    assert_token(&lexer, EL_TT_RBRACE, "}");
    assert_token(&lexer, EL_TT_SEMICOLON, ";");
    assert_token(&lexer, EL_TT_COLON, ":");
    assert_token(&lexer, EL_TT_DOUBLECOLON, "::");
    assert_token(&lexer, EL_TT_COMMA, ",");
    assert_token(&lexer, EL_TT_DOT, ".");
    assert_token(&lexer, EL_TT_HASH, "#");
    assert_token(&lexer, EL_TT_ELLIPSIS, "...");
    assert_token(&lexer, EL_TT_EOF, "");

    el_srcdoc_destroy(&doc);
}

Test(elash_lexer_happy, comments_skipped) {
    ElSourceDocument doc;
    el_srcdoc_init_from_str(&doc, EL_SV("foo // line comment\nbar /* block\ncomment */ baz"), EL_SV("test.ei"));

    ElLexer lexer;
    el_lexer_init(&lexer, &doc, EL_LEXER_FLAGS_DEFAULT | EL_LF_SKIP_COMMENTS);

    assert_token(&lexer, EL_TT_IDENT, "foo");
    assert_token(&lexer, EL_TT_IDENT, "bar");
    assert_token(&lexer, EL_TT_IDENT, "baz");
    assert_token(&lexer, EL_TT_EOF, "");

    el_srcdoc_destroy(&doc);
}

Test(elash_lexer_happy, comments_kept) {
    ElSourceDocument doc;
    el_srcdoc_init_from_str(&doc, EL_SV("foo // line comment\nbar /* block comment */ baz"), EL_SV("test.ei"));

    ElLexer lexer;
    el_lexer_init(&lexer, &doc, EL_LEXER_FLAGS_DEFAULT & ~EL_LF_SKIP_COMMENTS);

    assert_token(&lexer, EL_TT_IDENT, "foo");
    assert_token(&lexer, EL_TT_LINE_COMMENT, " line comment");
    assert_token(&lexer, EL_TT_IDENT, "bar");
    assert_token(&lexer, EL_TT_BLOCK_COMMENT, " block comment ");
    assert_token(&lexer, EL_TT_IDENT, "baz");
    assert_token(&lexer, EL_TT_EOF, "");

    el_srcdoc_destroy(&doc);
}
