#include <elash/diag/printer/console.h>
#include <elash/diag/engine.h>

#include <elash/srcdoc/srcdoc.h>
#include <elash/util/ansi.h>

void el_diag_console_printer_begin(ElDiagPrinter* self, FILE* out) {
    (void) self;
    (void) out;
}

ElAnsiStyle _el_diag_console_printer_get_style(ElDiagSeverity sev) {
    ElAnsiColor color = EL_ANSI_CLR_DEFAULT;
    switch (sev) {
    case EL_DIAG_ERROR: color = EL_ANSI_CLR_RED;            break;
    case EL_DIAG_WARN:  color = EL_ANSI_CLR_YELLOW;         break;
    case EL_DIAG_NOTE:  color = EL_ANSI_CLR_BRIGHT_MAGENTA; break;
    case EL_DIAG_TIP:   color = EL_ANSI_CLR_BRIGHT_GREEN;   break;
    }

    return (ElAnsiStyle) {
        .bg_color = EL_ANSI_CLR_DEFAULT,
        .fg_color = color,
        .dec = EL_ANSI_DEC_BOLD,
    };
}

void _el_diag_console_printer_print_sev(ElDiagSeverity sev, ElStringView cat, FILE* out) {
    ElStringView sev_str = el_diag_severity_to_string(sev);
    ElAnsiStyle style = _el_diag_console_printer_get_style(sev);
    bool ansi = el_ansi_is_supported(out);

    if (ansi) el_ansi_apply_style(style, out);
    fprintf(out, EL_SV_FMT"["EL_SV_FMT"]: ", EL_SV_FARG(sev_str), EL_SV_FARG(cat));
    if (ansi) el_ansi_reset_style(out);
}

void _el_diag_console_printer_print_loc(const ElSourceSpan* span, FILE* out) {
    if (span->doc) {
        fprintf(out, "at "EL_SV_FMT":%u:%u\n",
                EL_SV_FARG(span->doc->filename), span->start.line, span->start.column);
    }
}

static ElStringView _el_diag_console_printer_get_line_content(ElStringView content, uint offset) {
    uint start = offset;
    while (start > 0 && content.data[start - 1] != '\n' && content.data[start - 1] != '\r') {
        start--;
    }

    uint end = offset;
    while (end < content.len && content.data[end] != '\n' && content.data[end] != '\r') {
        end++;
    }

    return el_sv_slice(content, start, end);
}

static void _el_diag_console_printer_print_carets(FILE* out, ElStringView line, const ElSourceSpan* span, ElDiagSeverity sev) {
    fprintf(out, "   | ");

    uint start_col = span->start.column > 0 ? span->start.column : 1;
    uint end_col = span->end.column > 0 ? span->end.column : start_col;

    for (uint i = 0; i < start_col; i++) {
        if (i < line.len && line.data[i] == '\t') {
            fputc('\t', out);
        } else {
            fputc(' ', out);
        }
    }

    ElAnsiStyle style = _el_diag_console_printer_get_style(sev);
    bool ansi = el_ansi_is_supported(out);
    if (ansi) el_ansi_apply_style(style, out);

    uint caret_len = 1;
    if (span->end.line == span->start.line) {
        if (end_col > start_col) {
            caret_len = end_col - start_col;
        }
    } else {
        if (line.len >= (start_col - 1)) {
            caret_len = line.len - (start_col - 1);
        }
    }
    if (caret_len == 0) caret_len = 1;

    for (uint i = 0; i < caret_len; i++)
        fputc('^', out);

    if (ansi) el_ansi_reset_style(out);
    fputc('\n', out);
}

void _el_diag_console_printer_print_snippet(const ElSourceSpan* span, ElDiagSeverity sev, FILE* out) {
    if (span->doc == NULL) return;

    ElStringView content = el_srcdoc_content(span->doc);
    if (content.data == NULL || content.len == 0) return;

    ElStringView line = _el_diag_console_printer_get_line_content(content, span->start.offset);

    fprintf(out, "%2u | "EL_SV_FMT"\n", span->start.line, EL_SV_FARG(line));
    _el_diag_console_printer_print_carets(out, line, span, sev);
}

void el_diag_console_printer_print(ElDiagPrinter* self, FILE* out, const ElDiagnostic* diag) {
    (void) self;
    _el_diag_console_printer_print_sev(diag->sev, diag->category, out);
    el_sv_print(diag->formatted, out);
    fputc('\n', out);
    _el_diag_console_printer_print_loc(&diag->span, out);
    _el_diag_console_printer_print_snippet(&diag->span, diag->sev, out);

    for (ElDiagnosticHelp* help = diag->help_head; help != NULL; help = help->next) {
        bool ansi = el_ansi_is_supported(out);

        if (ansi) {
            ElAnsiStyle style = {
                .bg_color = EL_ANSI_CLR_DEFAULT,
                .fg_color = EL_ANSI_CLR_BRIGHT_CYAN,
                .dec = EL_ANSI_DEC_BOLD,
            };
            el_ansi_apply_style(style, out);
        }
        fputs("help: ", out);
        if (ansi) el_ansi_reset_style(out);

        el_sv_print(help->formatted, out);
        fputc('\n', out);
    }
}

static void print_diag_count(FILE *out, unsigned int count, ElDiagSeverity sev, bool ansi) {
    if (count == 0) return;
    ElAnsiStyle style = _el_diag_console_printer_get_style(sev);
    if (ansi) el_ansi_apply_style(style, out);
    fprintf(out, "%u %s%s", count, (sev == EL_DIAG_ERROR ? "error" : "warning"), (count == 1 ? "" : "s"));
    if (ansi) el_ansi_reset_style(out);
}

void el_diag_console_printer_summary(ElDiagPrinter* self, FILE* out, const ElDiagSummary* sum) {
    (void) self;
    if (sum->total_diagnostics != 0) {
        bool ansi = el_ansi_is_supported(out);
        fputs("Finished with ", out);

        print_diag_count(out, sum->total_errors, EL_DIAG_ERROR, ansi);
        if (sum->total_errors > 0 && sum->total_warnings > 0) fputs(" and ", out);
        print_diag_count(out, sum->total_warnings, EL_DIAG_WARN, ansi);
        fputs("\n", out);
    }
}

void el_diag_console_printer_finish(ElDiagPrinter* self, FILE* out) {
    (void) self;
    (void) out;
}
