#include <elash/diag/engine.h>

#include <elash/diag/template.h>
#include <elash/util/strbuf.h>
#include <elash/util/assert.h>

#include <elash/source/doc.h>

void el_diag_engine_init(ElDiagEngine* engine, ElDynArena* arena) {
    *engine = (ElDiagEngine) {
        .arena = arena,
    };
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

        if (entries[i].type == EL_DIAG_META_STR) {
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
    ElSourceSpan span, ElSourceLocInfo source,
    ElStringView template, ElDiagMeta meta,
    bool noignore
) {
    bool ignore = false;
    if (!noignore && (sev == EL_DIAG_WARN || sev == EL_DIAG_NOTE))
        for (ElSourceRange* r = span.ranges; r < span.ranges + span.count; ++r)
            ignore |= r->doc->is_system;

    engine->last_report_accepted = !ignore;
    if (ignore)
        return NULL;

    ElDiagnostic* diag = EL_DYNARENA_NEW_STRUCT(engine->arena, ElDiagnostic, {
        .sev = sev,
        .source = source,
        .category = category,
        .span = span,
        .help = {
            .head = NULL,
            .tail = NULL,
        },
    });

    _el_diag_format_message(engine, template, &meta, &diag->template, &diag->formatted);
    diag->meta = meta;

    if (diag->sev == EL_DIAG_ERROR) engine->summary.total_errors++;
    if (diag->sev == EL_DIAG_WARN)  engine->summary.total_warnings++;
    engine->summary.total_diagnostics++;

    diag->next = NULL;
    diag->prev = engine->diag.tail;

    if (engine->diag.tail != NULL) {
        engine->diag.tail->next = diag;
    } else {
        engine->diag.head = diag;
    }

    engine->diag.tail = diag;
    engine->diag.count++;
    return NULL;
}

void el_diag_help_impl(
    ElDiagEngine* engine, ElSourceLocInfo source,
    ElStringView template, ElDiagMeta meta
) {
    if (!engine->last_report_accepted) return;

    // just in case
    EL_ASSERT(engine->diag.tail != NULL, "no diagnostic to add help to");

    ElDiagnosticHelp* help = EL_DYNARENA_NEW_STRUCT(engine->arena, ElDiagnosticHelp, {
        .source = source,
        .meta = meta,
        .next = NULL,
    });

    _el_diag_format_message(engine, template, &meta, &help->template, &help->formatted);

    ElDiagnostic* dt = engine->diag.tail;
    if (dt->help.tail != NULL) {
        dt->help.tail->next = help;
    } else {
        dt->help.head = help;
    }
    dt->help.tail = help;
}

ElDiagSummary el_diag_engine_summary(const ElDiagEngine* engine) {
    return engine->summary;
}

void el_diag_engine_print(const ElDiagEngine* engine, ElDiagPrinter* printer, FILE* out) {
    printer->begin(printer, out);
    for (ElDiagnostic* diag = engine->diag.head; diag != NULL; diag = diag->next) {
        printer->print(printer, out, diag);
    }
    printer->summary(printer, out, &engine->summary);
    printer->finish(printer, out);
}
