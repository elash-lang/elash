#include <elash/diag/engine.h>

#include <elash/diag/template.h>
#include <elash/util/strbuf.h>
#include <elash/util/assert.h>

void el_diag_engine_init(ElDiagEngine* engine, ElDynArena* arena) {
    engine->arena = arena;
    engine->diag_count = 0;
    engine->diag_head = NULL;
    engine->diag_tail = NULL;
    engine->summary = (ElDiagSummary) { 0 };
}

void el_diag_engine_free(ElDiagEngine* engine) {
    // nothing for now
    (void) engine;
}

static ElDiagMeta _el_diag_clone_meta(ElDynArena* arena, ElDiagMeta meta) {
    if (meta.count == 0) {
        return (ElDiagMeta) { .entries = NULL, .count = 0 };
    }

    ElDiagMetaEntry* entries = EL_DYNARENA_NEW_ARR(arena, ElDiagMetaEntry, meta.count);
    for (usize i = 0; i < meta.count; i++) {
        entries[i].key = meta.entries[i].key;
        entries[i].type = meta.entries[i].type;
        entries[i].as = meta.entries[i].as;

        if (entries[i].type == EL_DIAG_META_STRING) {
            entries[i].as.string = el_dynarena_clone_sv(arena, meta.entries[i].as.string);
        }
    }

    return (ElDiagMeta) { .entries = entries, .count = meta.count };
}

static void _el_diag_format_message(
    ElDiagEngine* engine,
    ElStringView template, ElDiagMeta* meta,
    ElStringView* out_template, ElStringView* out_formatted
) {
    *meta = _el_diag_clone_meta(engine->arena, *meta);
    *out_template = el_dynarena_clone_sv(engine->arena, template);

    ElStringBuf formatted;
    el_strbuf_init(&formatted);
    if (el_diag_render_template(template, meta, &formatted)) {
        *out_formatted = el_dynarena_clone_sv(engine->arena, el_strbuf_view(&formatted));
    } else {
        *out_formatted = *out_template;
    }
    el_strbuf_destroy(&formatted);
}

void* el_diag_report_impl(
    ElDiagEngine* engine,
    ElDiagSeverity sev, ElStringView category,
    ElSourceSpan span,
    ElStringView template, ElDiagMeta meta
) {
    ElDiagnostic* diag = EL_DYNARENA_NEW_STRUCT(engine->arena, ElDiagnostic, {
        .sev = sev,
        .category = category,
        .span = span,
        .help_head = NULL,
        .help_tail = NULL,
    });

    _el_diag_format_message(engine, template, &meta, &diag->template, &diag->formatted);
    diag->meta = meta;

    if (diag->sev == EL_DIAG_ERROR) engine->summary.total_errors++;
    if (diag->sev == EL_DIAG_WARN)  engine->summary.total_warnings++;
    engine->summary.total_diagnostics++;

    diag->next = NULL;
    diag->prev = engine->diag_tail;

    if (engine->diag_tail) {
        engine->diag_tail->next = diag;
    } else {
        engine->diag_head = diag;
    }

    engine->diag_tail = diag;
    engine->diag_count++;
    return NULL;
}

void el_diag_help_impl(
    ElDiagEngine* engine,
    ElStringView template, ElDiagMeta meta
) {
    // just in case
    EL_ASSERT(engine->diag_tail != NULL, "no diagnostic to add help to");

    ElDiagnosticHelp* help = EL_DYNARENA_NEW(engine->arena, ElDiagnosticHelp);
    _el_diag_format_message(engine, template, &meta, &help->template, &help->formatted);
    help->meta = meta;
    help->next = NULL;

    if (engine->diag_tail->help_tail != NULL) {
        engine->diag_tail->help_tail->next = help;
    } else {
        engine->diag_tail->help_head = help;
    }
    engine->diag_tail->help_tail = help;
}

ElDiagSummary el_diag_engine_summary(const ElDiagEngine* engine) {
    return engine->summary;
}

void el_diag_engine_print(const ElDiagEngine* engine, ElDiagPrinter* printer, FILE* out) {
    printer->begin(printer, out);
    for (ElDiagnostic* diag = engine->diag_head; diag != NULL; diag = diag->next) {
        printer->print(printer, out, diag);
    }
    printer->summary(printer, out, &engine->summary);
    printer->finish(printer, out);
}
