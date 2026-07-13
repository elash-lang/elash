#pragma once

#include <elash/util/dynarena.h>

#include <elash/srcdoc/span.h>

#include <elash/diag/severity.h>
#include <elash/diag/printer.h>
#include <elash/diag/meta.h>

typedef struct ElDiagnosticHelp {
    ElStringView template;
    ElStringView formatted;
    ElDiagMeta meta;
    struct ElDiagnosticHelp* next;
} ElDiagnosticHelp;

typedef struct ElDiagnostic {
    ElDiagSeverity sev;
    ElStringView category;
    ElSourceSpan span;
    ElStringView formatted;
    ElStringView template;
    ElDiagMeta meta;

    ElDiagnosticHelp* help_head;
    ElDiagnosticHelp* help_tail;

    struct ElDiagnostic* prev;
    struct ElDiagnostic* next;
} ElDiagnostic;

typedef struct ElDiagSummary {
    uint total_diagnostics;
    uint total_errors;
    uint total_warnings;
} ElDiagSummary;

typedef struct ElDiagEngine {
    ElDynArena* arena;
    ElDiagnostic* diag_head;
    ElDiagnostic* diag_tail;
    usize diag_count;
    ElDiagSummary summary;
} ElDiagEngine;

void el_diag_engine_init(ElDiagEngine* engine, ElDynArena* arena);
void el_diag_engine_free(ElDiagEngine* engine);

void el_diag_report_impl(
    ElDiagEngine* engine,
    ElDiagSeverity sev, ElStringView category,
    ElSourceSpan span,
    ElStringView template, ElDiagMeta meta
);

void el_diag_help_impl(
    ElDiagEngine* engine,
    ElStringView template, ElDiagMeta meta
);

ElDiagSummary el_diag_engine_summary(const ElDiagEngine* engine);
void el_diag_engine_print(const ElDiagEngine* engine, ElDiagPrinter* printer, FILE* out);

static inline bool el_diag_engine_has_errors(const ElDiagEngine* engine) {
    return engine->summary.total_errors > 0;
}

#define el_diag_report(engine, sev, cat, span, template, ...) \
    el_diag_report_impl(engine, sev, EL_SV(cat), span, EL_SV(template), EL_DIAG_META(__VA_ARGS__))

#define el_diag_help(engine, template, ...) \
    el_diag_help_impl(engine, EL_SV(template), EL_DIAG_META(__VA_ARGS__))

