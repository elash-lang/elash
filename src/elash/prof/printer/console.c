#include "../prof-internals.h"

#include <elash/defs/sv.h>
#include <elash/util/ansi.h>

#include <stdio.h>
#include <string.h>

// two seems to be the best option, though 3 also looks good.
#define INDENT_SPACES 2

#define NAME_WIDTH    36
#define COUNT_WIDTH   7
#define TIME_WIDTH    10
#define PERCENT_WIDTH 8

#define SHOW_OTHER_TRESHOLD 1e-9

// WINAPI-grade coding
#define MIN(p, q) (((p) < (q)) ? (p) : (q))

static ElAnsiStyle bold_style = {
    .fg_color = EL_ANSI_CLR_DEFAULT,
    .bg_color = EL_ANSI_CLR_DEFAULT,
    .dec = EL_ANSI_DEC_BOLD,
};

static usize format_duration(char* buf, usize cap, double sec) {
    // NOLINTBEGIN(readability-magic-numbers)
    if (sec < 1e-9) return (usize)snprintf(buf, cap, "0ns");
    if (sec < 1e-6) return (usize)snprintf(buf, cap, "%.0fns", sec * 1e9);
    if (sec < 1e-3) return (usize)snprintf(buf, cap, "%.1fμs", sec * 1e6);
    if (sec < 1.0)  return (usize)snprintf(buf, cap, "%.2fms", sec * 1e3);
    // NOLINTEND(readability-magic-numbers)
    return (usize)snprintf(buf, cap, "%.3fs", sec);
}

static usize format_percent(char* buf, usize cap, double part, double whole) {
    if (whole <= 0.0) return (usize)snprintf(buf, cap, "   -   ");
    return (usize)snprintf(buf, cap, "%6.1f%%", (part / whole) * 100.0);
}

static void print_padded(FILE* out, int width, const char* text) {
    // made by ai; don't ask me how this works
    // fixes align issues with the μ character as it's non ansi
    // NOLINTBEGIN
    size_t len = strlen(text);
    size_t mu_count = 0;
    for (const char* p = text; *p != '\0'; p++) {
        if ((unsigned char)*p == 0xCE && (unsigned char)*(p+1) == 0xBC) {
            mu_count++;
            p++;
        }
    }
    int visual_len = (int)(len - mu_count);
    int padding = width - visual_len;
    if (padding < 0) padding = 0;
    // NOLINTEND
    fprintf(out, "%*s%s", padding, "", text);
}

#define PREFIX_BUF_SIZE 8 // I HATE clang-tidy's readability-magic-numbers
static void print_name(FILE* out, bool ansi, ElStringView name, int indent, bool is_stage) {
    char prefix[PREFIX_BUF_SIZE];
    usize prefix_len = MIN(sizeof(prefix) - 1,
                           (usize)indent * INDENT_SPACES);

    memset(prefix, ' ', prefix_len);
    prefix[prefix_len] = '\0';

    usize label_width = NAME_WIDTH - prefix_len;

    usize name_len = name.len;
    if (name_len > label_width) name_len = label_width;

    if (is_stage && ansi) {
        el_ansi_apply_style(bold_style, out);
    }

    uint pad = label_width - name_len;
    fprintf(out, "%s%.*s%*s", prefix, (int)name_len, name.data, pad, "");

    if (is_stage && ansi) {
        el_ansi_reset_style(out);
    }
}

#define STATS_BUF_SIZE 48
static void print_stat_cols(
    FILE* out, const ElProfStat* stat, double whole_wall
) {
    char buf[STATS_BUF_SIZE];

    format_duration(buf, sizeof(buf), stat->wall_sec);
    print_padded(out, TIME_WIDTH, buf);

    format_duration(buf, sizeof(buf), stat->user_sec);
    print_padded(out, TIME_WIDTH, buf);

    format_duration(buf, sizeof(buf), stat->sys_sec);
    print_padded(out, TIME_WIDTH, buf);

    format_percent(buf, sizeof(buf), stat->wall_sec, whole_wall);
    print_padded(out, PERCENT_WIDTH, buf);
}

static void print_sub_row(
    FILE* out, ElStringView name, int indent,
    uint count, const ElProfStat* stat, double stage_wall
) {
    print_name(out, false, name, indent, false);
    fprintf(out, " %*u", COUNT_WIDTH, count);
    fputc(' ', out);
    print_stat_cols(out, stat, stage_wall);
    fputc('\n', out);
}

static void print_stage_row(
    FILE* out, bool ansi, ElStringView name,
    const ElProfStat* stat, double total_wall
) {
    print_name(out, ansi, name, 0, true);
    fprintf(out, " %*s", COUNT_WIDTH, "");
    fputc(' ', out);
    print_stat_cols(out, stat, total_wall);
    fputc('\n', out);
}

static void print_header(FILE* out, bool ansi) {
    if (ansi) el_ansi_apply_style(bold_style, out);

    fprintf(out, "%-*s %*s", NAME_WIDTH, "Stage", COUNT_WIDTH, "Count");
    print_padded(out, TIME_WIDTH, "Wall");
    print_padded(out, TIME_WIDTH, "User");
    print_padded(out, TIME_WIDTH, "Sys");
    print_padded(out, PERCENT_WIDTH, "%");

    if (ansi) el_ansi_reset_style(out);
    fputc('\n', out);
}

static double total_wall_sec(ElProfState* prof) {
    double total = 0;
    for (ElProfStage* stage = prof->head; stage != NULL; stage = stage->next) {
        ElProfStat stat = el_prof_stage_stat(stage);
        total += stat.wall_sec;
    }
    return total;
}

void el_prof_print_console(ElProfState* prof, FILE* out) {
    if (prof->head == NULL) return;

    bool ansi = el_ansi_is_supported(out);
    double total_wall = total_wall_sec(prof);

    print_header(out, ansi);

    for (ElProfStage* stage = prof->head; stage != NULL; stage = stage->next) {
        ElProfStat stat = el_prof_stage_stat(stage);
        print_stage_row(out, ansi, stage->name, &stat, total_wall);

        for (ElProfSubstage* sub = stage->sub.head; sub != NULL; sub = sub->next) {
            print_sub_row(out, sub->name, 1, sub->entered_count, &sub->total, stat.wall_sec);
        }

        ElProfStat other = el_prof_substage_other(stage, &stat);
        if (other.wall_sec > SHOW_OTHER_TRESHOLD) {
            print_sub_row(out, EL_SV("(other)"), 1, 0, &other, stat.wall_sec);
        }

        fputc('\n', out);
    }
}
