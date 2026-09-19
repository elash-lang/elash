#pragma once

#include <elash/diag/printer.h>
#include <stdint.h>
#include <stdbool.h>

void el_diag_console_printer_begin(ElDiagPrinter* self, FILE* out);
void el_diag_console_printer_print(ElDiagPrinter* self, FILE* out, const ElDiagnostic* diag);
void el_diag_console_printer_summary(ElDiagPrinter* self, FILE* out, const ElDiagSummary* sum);
void el_diag_console_printer_finish(ElDiagPrinter* self, FILE* out);

static inline ElDiagPrinter el_diag_make_console_printer(bool debug) {
    return (ElDiagPrinter) {
        .begin = el_diag_console_printer_begin,
        .print = el_diag_console_printer_print,
        .summary = el_diag_console_printer_summary,
        .finish = el_diag_console_printer_finish,
        .ctx = (void*)(intptr_t)debug,
    };
}
