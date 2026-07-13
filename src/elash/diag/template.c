#include <elash/diag/template.h>
#include <elash/util/assert.h>

#include <stdio.h>

static bool _el_diag_render_meta_value(const ElDiagMetaEntry* entry, ElStringBuf* out) {
    switch (entry->type) {
    case EL_DIAG_META_INTEGER: {
        char buf[32]; // NOLINT
        int n = snprintf(buf, sizeof(buf), "%d", entry->as.integer);

        return el_strbuf_append(out, el_sv_from_data_and_len(buf, (usize)n));
    }
    case EL_DIAG_META_CHARACTER:
        return el_strbuf_append_char(out, entry->as.character);
    case EL_DIAG_META_STRING:
        return el_strbuf_append(out, entry->as.string);
    case EL_DIAG_META_TYPE:
        el_sema_format_type(entry->as.type, out);
        return true;
    }

    EL_UNREACHABLE_ENUM_VAL(ElDiagMetaType, entry->type);
}

static bool _el_diag_render_variable(ElStringView key, const ElDiagMeta* meta, ElStringBuf* out, bool* found) {
    *found = false;
    if (!meta) return true;

    for (usize i = 0; i < meta->count; i++) {
        if (el_sv_eql(key, el_sv_from_cstr(meta->entries[i].key))) {
            *found = true;
            return _el_diag_render_meta_value(&meta->entries[i], out);
        }
    }

    return true;
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity): i don't care
bool el_diag_render_template(ElStringView template, const ElDiagMeta* meta, ElStringBuf* out) {
    for (usize i = 0; i < template.len; i++) {
        if (template.data[i] == '$' && i + 1 < template.len && template.data[i + 1] == '{') {
            usize start = i + 2;
            usize end = start;

            while (end < template.len && template.data[end] != '}') {
                end++;
            }

            if (end < template.len) {
                ElStringView key = el_sv_slice(template, start, end);
                bool found;

                if (!_el_diag_render_variable(key, meta, out, &found)) {
                    return false;
                }

                EL_ASSERT(found, "unknown variable in diagnostic template string");

                i = end;
                continue;
            }
        }

        if (!el_strbuf_append_char(out, template.data[i])) {
            return false;
        }
    }

    return true;
}
