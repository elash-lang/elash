#pragma once

#include <elash/diag/printer.h>

void el_diag_jsonl_printer_begin(ElDiagPrinter* self, FILE* out);
void el_diag_jsonl_printer_print(ElDiagPrinter* self, FILE* out, const ElDiagnostic* diag);
void el_diag_jsonl_printer_summary(ElDiagPrinter* self, FILE* out, const ElDiagSummary* sum);
void el_diag_jsonl_printer_finish(ElDiagPrinter* self, FILE* out);

static inline ElDiagPrinter el_diag_make_jsonl_printer() {
    return (ElDiagPrinter) {
        .begin = el_diag_jsonl_printer_begin,
        .print = el_diag_jsonl_printer_print,
        .summary = el_diag_jsonl_printer_summary,
        .finish = el_diag_jsonl_printer_finish,
        .ctx = NULL,
    };
}

