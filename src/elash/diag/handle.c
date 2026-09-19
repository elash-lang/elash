#include <elash/diag/handle.h>

#include <elash/util/assert.h>

void el_diag_handle_lexer_error(ElDiagEngine* engine, const ElLexerResult* err) {
    switch (err->code) {
    case EL_LEXERR_SUCCESS:
    case _EL_LEXERR_COUNT:
        return;

    case EL_LEXERR_INTERNAL_ERROR:
        el_diag_report(engine, EL_DIAG_ERROR, "internal", err->span, "internal lexer error");
        return;
    case EL_LEXERR_INVALID_ARG:
        el_diag_report(engine, EL_DIAG_ERROR, "internal", err->span, "internal lexer error (invalid args passed)");
        return;
    case EL_LEXERR_INVALID_ESCAPE:
        el_diag_report(
            engine, EL_DIAG_ERROR, "syntax.invalid-escape",
            err->span, "invalid escape: '\\${char}'",
            EL_DIAG_CHAR("char", err->error_details.ch),
        ); return;
    case EL_LEXERR_INVALID_NUMBER:
        el_diag_report(
            engine, EL_DIAG_ERROR, "syntax.invalid-number",
            err->span, "invalid number literal: '${lit}'",
            EL_DIAG_STRING("lit", el_srcspan_to_sv(err->span)),
        ); return;
    case EL_LEXERR_INVALID_UNICODE:
        el_diag_report(
            engine, EL_DIAG_ERROR, "syntax.invalid-unicode",
            err->span, "invalid unicode codepoint",
        ); return;
    case EL_LEXERR_UNEXPECTED_CHAR:
        el_diag_report(
            engine, EL_DIAG_ERROR, "syntax.unexpected-char",
            err->span, "unexpected character: '${char}'",
            EL_DIAG_CHAR("char", err->error_details.ch),
        ); return;
    case EL_LEXERR_UNTERM_STRING:
        el_diag_report(
            engine, EL_DIAG_ERROR, "syntax.unterminated",
            err->span, "unterminated string literal",
        ); return;
    case EL_LEXERR_UNTERM_CHAR:
        el_diag_report(
            engine, EL_DIAG_ERROR, "syntax.unterminated",
            err->span, "unterminated char literal",
        ); return;
    case EL_LEXERR_UNTERM_COMMENT:
        el_diag_report(
            engine, EL_DIAG_ERROR, "syntax.unterminated",
            err->span, "unterminated multi-line comment",
        ); return;
    }
    EL_UNREACHABLE_ENUM_VAL(ElLexerStatus, err->code);
}
