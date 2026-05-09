#include <criterion/criterion.h>

#include <elash/lexer/lexer.h>
#include <elash/srcdoc/srcdoc.h>

static void assert_lexer_error(const char* input, ElLexerErrorCode expected_error) {
    ElSourceDocument doc;
    el_srcdoc_init_from_str(&doc, el_sv_from_cstr(input), EL_SV("test.ela"));

    ElLexer lexer;
    el_lexer_init(&lexer, &doc, EL_LEXER_FLAGS_DEFAULT);

    ElToken tok;
    ElLexerErrorCode err = el_lexer_next_token(&lexer, &tok);
    
    ElStringView err_sv = el_lexer_err_code_to_string(err);
    ElStringView expected_err_sv = el_lexer_err_code_to_string(expected_error);
    
    cr_assert_eq(err, expected_error, 
                 "Expected error " EL_SV_FMT ", got " EL_SV_FMT " for input '%s'", 
                 EL_SV_FARG(expected_err_sv), EL_SV_FARG(err_sv), input);

    el_srcdoc_destroy(&doc);
}

Test(lexer_unhappy, unexpected_char) {
    assert_lexer_error("@", EL_LEXERR_UNEXPECTED_CHAR);
}

Test(lexer_unhappy, unterminated_string) {
    assert_lexer_error("\"hello", EL_LEXERR_UNTERM_STRING);
}

Test(lexer_unhappy, unterminated_char) {
    assert_lexer_error("'a", EL_LEXERR_UNTERM_CHAR);
}

Test(lexer_unhappy, unterminated_comment) {
    assert_lexer_error("/* comment", EL_LEXERR_UNTERM_COMMENT);
}

