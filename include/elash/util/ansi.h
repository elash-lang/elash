#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <elash/util/strbuf.h>

typedef enum ElAnsiColor {
    EL_ANSI_CLR_BLACK          = 30,
    EL_ANSI_CLR_RED,
    EL_ANSI_CLR_GREEN,
    EL_ANSI_CLR_YELLOW,
    EL_ANSI_CLR_BLUE,
    EL_ANSI_CLR_MAGENTA,
    EL_ANSI_CLR_CYAN,
    EL_ANSI_CLR_WHITE,
    EL_ANSI_CLR_BRIGHT_BLACK   = 90,
    EL_ANSI_CLR_BRIGHT_RED,
    EL_ANSI_CLR_BRIGHT_GREEN,
    EL_ANSI_CLR_BRIGHT_YELLOW,
    EL_ANSI_CLR_BRIGHT_BLUE,
    EL_ANSI_CLR_BRIGHT_MAGENTA,
    EL_ANSI_CLR_BRIGHT_CYAN,
    EL_ANSI_CLR_BRIGHT_WHITE,
    EL_ANSI_CLR_DEFAULT        = 39,
} ElAnsiColor;

typedef enum ElAnsiDecoration {
    EL_ANSI_DEC_NONE        = 0,
    EL_ANSI_DEC_BOLD        = 1 << 0,
    EL_ANSI_DEC_ITALIC      = 1 << 1,
    EL_ANSI_DEC_UNDERLINE   = 1 << 2,
} ElAnsiDecoration;

typedef struct ElAnsiStyle {
    ElAnsiColor bg_color;
    ElAnsiColor fg_color;
    ElAnsiDecoration dec;
} ElAnsiStyle;

#define EL_ANSI_STYLE_DEFAULT ((ElAnsiStyle) { \
        .bg_color = EL_ANSI_CLR_DEFAULT,       \
        .fg_color = EL_ANSI_CLR_DEFAULT,       \
        .dec = EL_ANSI_DEC_NONE                \
    })

typedef enum ElAnsiPref {
    EL_ANSI_DISABLED,
    EL_ANSI_AUTO,
    EL_ANSI_ENABLED,
} ElAnsiPref;

typedef enum ElAnsiStream {
    EL_ANSI_STREAM_UNKNOWN = 0,
    EL_ANSI_STREAM_STDOUT,
    EL_ANSI_STREAM_STDERR,
} ElAnsiStream;

void el_ansi_init(ElAnsiPref pref);

ElAnsiStream el_ansi_stream_from_file(FILE* out);

bool el_ansi_is_supported(FILE* out);
bool el_ansi_is_stream_supported(ElAnsiStream stream);

void el_ansi_apply_style(ElAnsiStyle style, FILE* out);
void el_ansi_reset_style(FILE* out);

void el_ansi_append_style(ElStringBuf* sb, ElAnsiStyle style, FILE* out);
void el_ansi_append_reset(ElStringBuf* sb, FILE* out);
