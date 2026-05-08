#pragma once

#include <elash/defs/int-types.h>

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

typedef struct ElStringView {
    const char* data;
    usize len;
} ElStringView;

#define EL_SV_NULL ((ElStringView) { .data = NULL, .len = 0 })
#define EL_SV(STRING_LITERAL) ((ElStringView) { .data = (STRING_LITERAL), .len = sizeof(STRING_LITERAL) - 1 })

#define EL_SV_FMT "%.*s"
#define EL_SV_FARG(SV) \
    ((int)((SV).data ? (SV).len : 5)), \
    ((SV).data ? (SV).data : "(nil)")

static inline ulong el_sv_print(ElStringView sv, FILE* out) {
    return fwrite(sv.data, 1, sv.len, out);
}

static inline ElStringView el_sv_from_cstr(const char* cstr) {
    return (ElStringView) { .data = cstr, .len = strlen(cstr) };
}

static inline ElStringView el_sv_from_data_and_len(const char* data, usize len) {
    return (ElStringView) { .data = data, .len = len };
}

static inline bool el_sv_is_null(ElStringView sv) {
    return sv.data == NULL;
}

static inline bool el_sv_eql(ElStringView lhs, ElStringView rhs) {
    if (lhs.len != rhs.len) return false;
    if (lhs.data == rhs.data) return true;
    return memcmp(lhs.data, rhs.data, lhs.len) == 0;
}

static inline bool el_sv_starts_with(ElStringView sv, ElStringView prefix) {
    if (prefix.len > sv.len) return false;
    return memcmp(sv.data, prefix.data, prefix.len) == 0;
}

static inline bool el_sv_ends_with(ElStringView sv, ElStringView suffix) {
    if (suffix.len > sv.len) return false;
    if (suffix.len == sv.len) return memcmp(sv.data, suffix.data, sv.len) == 0;

    return memcmp(sv.data + (sv.len - suffix.len), suffix.data, suffix.len) == 0;
}

static inline ElStringView el_sv_trim_prefix(ElStringView sv, ElStringView prefix) {
    if (!el_sv_starts_with(sv, prefix)) return sv;
    return (ElStringView) { .data = sv.data + prefix.len, .len = sv.len - prefix.len };
}

static inline ElStringView el_sv_trim_suffix(ElStringView sv, ElStringView suffix) {
    if (!el_sv_ends_with(sv, suffix)) return sv;
    return (ElStringView) { .data = sv.data, .len = suffix.len };
}

static inline ElStringView el_sv_trim_prefix_or_null(ElStringView sv, ElStringView prefix) {
    if (!el_sv_starts_with(sv, prefix)) return EL_SV_NULL;
    return (ElStringView) { .data = sv.data + prefix.len, .len = sv.len - prefix.len };
}

static inline ElStringView el_sv_trim_suffix_or_null(ElStringView sv, ElStringView suffix) {
    if (!el_sv_ends_with(sv, suffix)) return EL_SV_NULL;
    return (ElStringView) { .data = sv.data, .len = suffix.len };
}

static inline ElStringView el_sv_slice(ElStringView sv, usize start, usize end) {
    if (sv.data == NULL || start > end || end > sv.len) {
        return EL_SV_NULL;
    }

    return (ElStringView) { .data = sv.data + start, .len = end - start };
}

static inline ElStringView el_sv_window(ElStringView sv, usize start, usize len) {
    if (start >= sv.len) return (ElStringView) { 0 };
    if (start + len > sv.len) len = sv.len - start;

    return (ElStringView) { .data = sv.data + start, .len = len };
}
