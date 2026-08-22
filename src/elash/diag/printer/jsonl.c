#include <elash/diag/printer/jsonl.h>
#include <elash/diag/engine.h>

#include <elash/util/assert.h>
#include <elash/util/strbuf.h>
#include <elash/hir/type.h>

#define MIN_PRINTABLE 0x20
static void escape_json_str(FILE* out, ElStringView sv) {
    fputc('"', out);
    for (const char* c = sv.data; c < sv.data + sv.len; ++c) {
        switch (*c) {
        case '"':  fputs("\\\"", out); break;
        case '\\': fputs("\\\\", out); break;
        case '\b': fputs("\\b", out);  break;
        case '\f': fputs("\\f", out);  break;
        case '\n': fputs("\\n", out);  break;
        case '\r': fputs("\\r", out);  break;
        case '\t': fputs("\\t", out);  break;
        default:
            if ((unsigned char)*c < MIN_PRINTABLE) {
                fprintf(out, "\\u%04x", *c);
            } else {
                fputc(*c, out);
            }
            break;
        }
    }
    fputc('"', out);
}

void el_diag_jsonl_printer_begin(ElDiagPrinter* self, FILE* out) {
    (void)self;
    (void)out;
}

static void dump_src_span(FILE* out, const ElSourceSpan* span) {
    fputs("{", out);
    fprintf(out, "\"count\":%u", span->count);
    fputs(",\"ranges\":[", out);
    for (int i = 0; i < span->count; ++i) {
        if (i > 0) fputc(',', out);
        const ElSourceRange* range = &span->ranges[i];
        fprintf(out, "{\"start\":{\"line\":%u,\"column\":%u,\"offset\":%u},",
                range->start.line, range->start.column, range->start.offset);
        fprintf(out, "\"end\":{\"line\":%u,\"column\":%u,\"offset\":%u}}",
                range->end.line, range->end.column, range->end.offset);
    }
    fputs("]}", out);
}

static void dump_token(FILE* out, const ElToken* token) {
    fputc('{', out);

    fputs("\"type\":", out);
    escape_json_str(out, el_token_type_to_string(token->type));

    fputs(",\"lexeme\":", out);
    escape_json_str(out, token->lexeme);

    fputs(",\"span\":", out);
    dump_src_span(out, &token->span);

    fputc('}', out);
}

static void dump_diag_meta(const ElDiagMetaEntry* entry, FILE* out) {
    switch (entry->type) {
    case EL_DIAG_META_TYPE: {
        ElStringBuf sb;
        el_strbuf_init(&sb);
        el_sema_format_type(entry->as.type, &sb);
        escape_json_str(out, el_strbuf_view(&sb));
        el_strbuf_destroy(&sb);
        return;
    }
    case EL_DIAG_META_STR:
        escape_json_str(out, entry->as.string);
        return;
    case EL_DIAG_META_INT:
        fprintf(out, "%d", entry->as.integer);
        return;
    case EL_DIAG_META_CHAR:
        escape_json_str(out, el_sv_from_char(&entry->as.character));
        return;
    case EL_DIAG_META_TOK:
        dump_token(out, &entry->as.token);
        return;
    }

    EL_UNREACHABLE_ENUM_VAL(ElDiagMetaType, entry->type);
}

void el_diag_jsonl_printer_print(ElDiagPrinter* self, FILE* out, const ElDiagnostic* diag) {
    (void)self;
    fputc('{', out);
    fputs("\"type\":\"diag\",", out);

    // basic info
    fputs("\"severity\":", out);
    escape_json_str(out, el_diag_severity_to_string(diag->sev));

    fputs(",\"category\":", out);
    if (diag->category.len == 0) {
        fputs("null", out);
    } else {
        escape_json_str(out, diag->category);
    }
    fputc(',', out);

    fputs("\"span\":", out);
    dump_src_span(out, &diag->span);

    fputs(",\"formatted\":", out);
    escape_json_str(out, diag->formatted);

    fputs(",\"template\":", out);
    escape_json_str(out, diag->template);

    // template meta
    fputs(",\"meta\":{", out);
    for (usize i = 0; i < diag->meta.count; ++i) {
        if (i > 0) fputc(',', out);

        const ElDiagMetaEntry* entry = &diag->meta.entries[i];

        // SAFETY: diagnostic meta keys are hard coded into the compiler and they
        //         never contain any user input so we don't need to escape them
        fprintf(out, "\"%s\":", entry->key);
        dump_diag_meta(entry, out);
    }
    fputc('}', out);

    // help messages
    fputs(",\"help\":[", out);
    for (ElDiagnosticHelp* help = diag->help_head; help; help = help->next) {
        if (help != diag->help_head) fputc(',', out);
        fputc('{', out);
        fputs("\"template\":", out);
        escape_json_str(out, help->template);
        fputs(",\"formatted\":", out);
        escape_json_str(out, help->formatted);
        fputc('}', out);
    }
    fputc(']', out);

    fputs("}\n", out);
}

void el_diag_jsonl_printer_summary(ElDiagPrinter* self, FILE* out, const ElDiagSummary* sum) {
    (void)self;
    fprintf(out, "{\"type\":\"summary\",\"total-diagnostics\":%u,\"total-errors\":%u,\"total-warnings\":%u}\n",
            sum->total_diagnostics, sum->total_errors, sum->total_warnings);
}

void el_diag_jsonl_printer_finish(ElDiagPrinter* self, FILE* out) {
    (void)self;
    (void)out;
}
