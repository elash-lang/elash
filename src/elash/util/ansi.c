#define _POSIX_C_SOURCE 200809L // for fileno
#include <elash/util/ansi.h>
#include <elash/util/assert.h>

#include <signal.h>
#include <stdio.h>

#define ANSI_BG_OFFSET 10

#ifdef _WIN32
    #include <io.h>
    #define ISATTY _isatty
    #define FILENO _fileno
    #define WRITE _write
#else
    #include <unistd.h>
    #define ISATTY isatty
    #define FILENO fileno
    #define WRITE write
#endif

static volatile _Atomic sig_atomic_t ansi_support = 0;

void el_ansi_init(ElAnsiPref pref) {
    bool stdout_supp = false;
    bool stderr_supp = false;

    switch (pref) {
    case EL_ANSI_DISABLED:
        stdout_supp = false;
        stderr_supp = false;
        break;
    case EL_ANSI_ENABLED:
        stdout_supp = true;
        stderr_supp = true;
        break;
    case EL_ANSI_AUTO:
        stdout_supp = ISATTY(FILENO(stdout)) != 0;
        stderr_supp = ISATTY(FILENO(stderr)) != 0;
        break;
    }

    ansi_support = 0;
    if (stdout_supp) ansi_support |= EL_ANSI_STREAM_STDOUT;
    if (stderr_supp) ansi_support |= EL_ANSI_STREAM_STDERR;
}

ElAnsiStream el_ansi_stream_from_file(FILE* out) {
    if (out == stdout) return EL_ANSI_STREAM_STDOUT;
    if (out == stderr) return EL_ANSI_STREAM_STDERR;
    return EL_ANSI_STREAM_UNKNOWN;
}

bool el_ansi_is_stream_supported(ElAnsiStream stream) {
    switch (stream) {
    case EL_ANSI_STREAM_STDOUT:  return ansi_support & EL_ANSI_STREAM_STDOUT;
    case EL_ANSI_STREAM_STDERR:  return ansi_support & EL_ANSI_STREAM_STDERR;
    case EL_ANSI_STREAM_UNKNOWN: return false;
    }
    return false;
}

bool el_ansi_is_supported(FILE* out) {
    ElAnsiStream stream = el_ansi_stream_from_file(out);
    return el_ansi_is_stream_supported(stream);
}

void el_ansi_apply_style(ElAnsiStyle style, FILE* out) {
    if (!el_ansi_is_supported(out)) return;

    // start with reset to ensure a clean state.
    fprintf(out, "\033[0");

    if (style.dec & EL_ANSI_DEC_BOLD)
        fprintf(out, ";1");
    if (style.dec & EL_ANSI_DEC_ITALIC)
        fprintf(out, ";3");
    if (style.dec & EL_ANSI_DEC_UNDERLINE)
        fprintf(out, ";4");

    if (style.fg_color != EL_ANSI_CLR_DEFAULT) {
        fprintf(out, ";%d", (int)style.fg_color);
    }
    if (style.bg_color != EL_ANSI_CLR_DEFAULT) {
        fprintf(out, ";%d", (int)style.bg_color + ANSI_BG_OFFSET);
    }

    fprintf(out, "m");
}

void el_ansi_reset_style(FILE* out) {
    if (el_ansi_is_supported(out)) {
        fputs("\033[0m", out);
    }
}

void el_ansi_append_style(ElStringBuf* sb, ElAnsiStyle style, FILE* out) {
    if (!el_ansi_is_supported(out)) return;

    EL_STRBUF_APPEND(sb, "\033[0");

    if (style.dec & EL_ANSI_DEC_BOLD)
        EL_STRBUF_APPEND(sb, ";1");
    if (style.dec & EL_ANSI_DEC_ITALIC)
        EL_STRBUF_APPEND(sb, ";3");
    if (style.dec & EL_ANSI_DEC_UNDERLINE)
        EL_STRBUF_APPEND(sb, ";4");

    if (style.fg_color != EL_ANSI_CLR_DEFAULT) {
        el_strbuf_appendf(sb, ";%d", (int)style.fg_color);
    }
    if (style.bg_color != EL_ANSI_CLR_DEFAULT) {
        el_strbuf_appendf(sb, ";%d", (int)style.bg_color + ANSI_BG_OFFSET);
    }

    el_strbuf_append_char(sb, 'm');
}

void el_ansi_append_reset(ElStringBuf* sb, FILE* out) {
    if (!el_ansi_is_supported(out)) return;
    EL_STRBUF_APPEND(sb, "\033[0m");
}
