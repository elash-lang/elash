#pragma once

#include <elash/util/dynarena.h>

#include <elash/source/span.h>
#include <elash/defs/locinfo.h>

#include <elash/diag/severity.h>
#include <elash/diag/printer.h>
#include <elash/diag/meta.h>

typedef struct ElDiagnosticHelp ElDiagnosticHelp;
struct ElDiagnosticHelp {
    ElStringView template;
    ElStringView formatted;
    ElDiagMeta meta;

    ElSourceLocInfo source;
    ElDiagnosticHelp* next;
};

typedef struct ElDiagnostic {
    ElDiagSeverity sev;
    ElStringView category;
    ElSourceSpan span;
    ElStringView formatted;
    ElStringView template;
    ElDiagMeta meta;

    struct {
        ElDiagnosticHelp* head;
        ElDiagnosticHelp* tail;
    } help;

    ElSourceLocInfo source;

    ElDiagnostic* prev;
    ElDiagnostic* next;
} ElDiagnostic;

typedef struct ElDiagSummary {
    uint total_diagnostics;
    uint total_errors;
    uint total_warnings;
} ElDiagSummary;

typedef struct ElDiagEngine {
    ElDynArena* arena;
    struct {
        ElDiagnostic* head;
        ElDiagnostic* tail;
        usize count;
    } diag;

    bool last_report_accepted;
    ElDiagSummary summary;
} ElDiagEngine;

void el_diag_engine_init(ElDiagEngine* engine, ElDynArena* arena);
void el_diag_engine_free(ElDiagEngine* engine);

// always returns NULL
// ugly but allows this pattern
// return el_diag_report(...);
void* el_diag_report_impl(
    ElDiagEngine* engine,
    ElDiagSeverity sev, ElStringView category,
    ElSourceSpan span, ElSourceLocInfo source,
    ElStringView template, ElDiagMeta meta,
    bool noignore
);

void el_diag_help_impl(
    ElDiagEngine* engine, ElSourceLocInfo source,
    ElStringView template, ElDiagMeta meta
);

ElDiagSummary el_diag_engine_summary(const ElDiagEngine* engine);
void el_diag_engine_print(const ElDiagEngine* engine, ElDiagPrinter* printer, FILE* out);

static inline bool el_diag_engine_has_errors(const ElDiagEngine* engine) {
    return engine->summary.total_errors > 0;
}

#define el_diag_report(engine, sev, cat, span, template, ...) \
    el_diag_report_impl(engine, sev, EL_SV(cat), span, EL_SRCLOC_INFO, EL_SV(template), EL_DIAG_META(__VA_ARGS__), false)

#define el_diag_report_nocat(engine, sev, span, template, ...) \
    el_diag_report_impl(engine, sev, EL_SV_NULL, span, EL_SRCLOC_INFO, EL_SV(template), EL_DIAG_META(__VA_ARGS__), false)

#define el_diag_report_ex(engine, noignore, sev, cat, span, template, ...) \
    el_diag_report_impl(engine, sev, EL_SV(cat), span, EL_SRCLOC_INFO, template, EL_DIAG_META(__VA_ARGS__), noignore)

#define el_diag_report_ex_nocat(engine, noignore, sev, span, template, ...) \
    el_diag_report_impl(engine, sev, EL_SV_NULL, span, EL_SRCLOC_INFO, template, EL_DIAG_META(__VA_ARGS__), noignore)

#define el_diag_help(engine, template, ...) \
    el_diag_help_impl(engine, EL_SRCLOC_INFO, EL_SV(template), EL_DIAG_META(__VA_ARGS__))
