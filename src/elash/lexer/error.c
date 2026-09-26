#include <elash/lexer/error.h>

#include <elash/defs/sv.h>
#include <elash/util/alloc.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static ElStringView EL_LEXERR_to_string_map[] = {
    [EL_LEXERR_SUCCESS] = EL_SV("SUCCESS"),
    [EL_LEXERR_UNEXPECTED_CHAR] = EL_SV("UNEXPECTED_CHAR"),
    [EL_LEXERR_INVALID_ESCAPE] = EL_SV("INVALID_ESCAPE"),
    [EL_LEXERR_UNTERM_COMMENT] = EL_SV("UNTERM_COMMENT"),
    [EL_LEXERR_UNTERM_STRING] = EL_SV("UNTERM_STRING"),
    [EL_LEXERR_UNTERM_CHAR] = EL_SV("UNTERM_CHAR"),
    [EL_LEXERR_INVALID_NUMBER] = EL_SV("INVALID_NUMBER"),
    [EL_LEXERR_INVALID_UNICODE] = EL_SV("INVALID_UNICODE"),
    [EL_LEXERR_INTERNAL_ERROR] = EL_SV("INTERNAL_ERROR"),
    [EL_LEXERR_INVALID_ARG] = EL_SV("INVALID_ARG"),
};

ElStringView el_lexer_err_code_to_string(ElLexerStatus code) {
    if (code < 0 || code >= _EL_LEXERR_COUNT) return EL_SV_NULL;
    ElStringView s = EL_LEXERR_to_string_map[code];
    if (s.data == NULL) return EL_SV("UNKNOWN_ERROR");
    return s;
}

usize el_lexer_result_to_string(ElLexerResult r, char** out) {
    ElStringView error_code_str = el_lexer_err_code_to_string(r.code);

    if (r.code == EL_LEXERR_SUCCESS) {
        *out = EL_NEW_ARR(char, error_code_str.len + 1);
        memcpy(*out, error_code_str.data, error_code_str.len);
        (*out)[error_code_str.len] = '\0';
        return error_code_str.len;
    }

    int loc_len = 0;
    if (r.code != EL_LEXERR_SUCCESS) {
        loc_len = snprintf(NULL, 0, " (%u:%u)",
            // line and column are 0-indexed but we want 1-indexed numbers
            r.span.ranges[0].start.line + 1, r.span.ranges[0].start.column + 1
        );
        if (loc_len < 0) loc_len = 0;
    }

    int char_len = 0;
    if (r.code == EL_LEXERR_UNEXPECTED_CHAR || r.code == EL_LEXERR_INVALID_ESCAPE) {
        char_len = snprintf(NULL, 0, " ('%c')", r.error_details.ch);
        if (char_len < 0) char_len = 0;
    }

    const usize full_len = error_code_str.len + (usize)loc_len + (usize)char_len;
    *out = EL_NEW_ARR(char, full_len + 1);

    char* p = *out;
    memcpy(p, error_code_str.data, error_code_str.len);
    p += error_code_str.len;

    if (loc_len > 0) {
        p += snprintf(p, (usize)loc_len + 1, " (%u:%u)",
            // line and column are 0-indexed but we want 1-indexed numbers
            r.span.ranges[0].start.line + 1, r.span.ranges[0].start.column + 1
        );
    }

    if (char_len > 0) {
        p += snprintf(p, (usize)char_len + 1, " ('%c')", r.error_details.ch);
    }

    *p = '\0';

    return full_len;
}

usize el_lexer_result_print(ElLexerResult r, FILE* out) {
    ulong written = 0;

    if (r.code != EL_LEXERR_SUCCESS) {
        written += fprintf(
            out, "(%u:%u): ",
            r.span.ranges[0].start.line + 1, r.span.ranges[0].start.column + 1
        );
    }

    written += el_sv_print(el_lexer_err_code_to_string(r.code), out);

    if (r.code == EL_LEXERR_UNEXPECTED_CHAR || r.code == EL_LEXERR_INVALID_ESCAPE) {
        if (isprint((unsigned char)r.error_details.ch)) {
            written += fprintf(out, " ('%c')", r.error_details.ch);
        } else {
            written += fprintf(out, " ('0x%02X')", r.error_details.ch);
        }
    }

    written += fprintf(out, "\n");
    return (written > 0) ? (usize)written : 0;
}

usize el_lexer_result_format(ElLexerResult r, usize n, char buf[static n]) {
    char* s = NULL;
    usize len = el_lexer_result_to_string(r, &s);

    // NOLINTNEXTLINE
    usize to_copy = (len < n) ? len : (n > 0 ? n - 1 : 0);
    if (n > 0) {
        memcpy(buf, s, to_copy);
        buf[to_copy] = '\0';
    }

    el_free(s);
    return to_copy;
}
