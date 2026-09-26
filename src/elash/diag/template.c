#include <elash/diag/template.h>
#include <elash/util/assert.h>

static void _el_diag_render_meta_value(const ElDiagMetaEntry* entry, ElStringBuf* out) {
    switch (entry->type) {
    case EL_DIAG_META_INT: {
        return el_strbuf_appendf(out, "%i", entry->as.integer);
    }
    case EL_DIAG_META_CHAR:
        return el_strbuf_append_char(out, entry->as.character);
    case EL_DIAG_META_STR:
        return el_strbuf_append(out, entry->as.string);
    case EL_DIAG_META_TYPE:
        return el_hir_format_type(entry->as.type, out);
    case EL_DIAG_META_TOK:
        return el_strbuf_append(out,
                el_token_type_format(entry->as.token.type));
    }

    EL_UNREACHABLE_ENUM_VAL(ElDiagMetaType, entry->type);
}

static void _el_diag_render_variable(ElStringView key, const ElDiagMeta* meta, ElStringBuf* out, bool* found) {
    *found = false;
    if (meta == NULL) return;

    for (usize i = 0; i < meta->count; i++) {
        if (el_sv_eql(key, el_sv_from_cstr(meta->entries[i].key))) {
            *found = true;
            _el_diag_render_meta_value(&meta->entries[i], out);
        }
    }
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity): i don't care
void el_diag_render_template(ElStringView template, const ElDiagMeta* meta, ElStringBuf* out) {
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

                _el_diag_render_variable(key, meta, out, &found);
                EL_ASSERT(found, "unknown variable in diagnostic template string");

                i = end;
                continue;
            }
        }

        el_strbuf_append_char(out, template.data[i]);
    }
}
