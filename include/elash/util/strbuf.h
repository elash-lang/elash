#pragma once

#include <elash/defs/sv.h>
#include <elash/defs/attr.h>

typedef struct ElStringBuf {
    char* data;
    usize len;
    usize cap;
} ElStringBuf;

bool el_strbuf_init(ElStringBuf* sb);
bool el_strbuf_init_with_cap(ElStringBuf* sb, usize init_cap);
bool el_strbuf_init_from(ElStringBuf* sb, ElStringView sv);
bool el_strbuf_init_from_cstr(ElStringBuf* sb, const char* cstr);
void el_strbuf_destroy(ElStringBuf* sb);

bool el_strbuf_copy(const ElStringBuf* src, ElStringBuf* dst);
void el_strbuf_move(ElStringBuf* src, ElStringBuf* dst);

static inline ElStringView el_strbuf_view(const ElStringBuf* sb) {
    return el_sv_from_data_and_len(sb->data, sb->len);
}

bool el_strbuf_resize(ElStringBuf* sb, usize new_len);
bool el_strbuf_reserve(ElStringBuf* sb, usize min_cap);
bool el_strbuf_reserve_exact(ElStringBuf* sb, usize new_cap);

void el_strbuf_clear(ElStringBuf* sb);

bool el_strbuf_append(ElStringBuf* sb, ElStringView sv);
bool el_strbuf_append_char(ElStringBuf* sb, char c);

bool el_strbuf_appendf(ElStringBuf* sb, const char* fmt, ...) EL_ATTR_PRINTF_LIKE(2, 3);

static inline bool el_strbuf_append_cstr(ElStringBuf* sb, const char* cstr) {
    return el_strbuf_append(sb, el_sv_from_cstr(cstr));
}
static inline bool el_strbuf_append_buf(ElStringBuf* sb, const ElStringBuf* other) {
    return el_strbuf_append(sb, el_strbuf_view(other));
}

static inline bool el_strbuf_eql_to(const ElStringBuf* lhs, ElStringView rhs) {
    return el_sv_eql(el_strbuf_view(lhs), rhs);
}
static inline bool el_strbuf_eql_to_cstr(const ElStringBuf* lhs, const char* rhs) {
    return el_strbuf_eql_to(lhs, el_sv_from_cstr(rhs));
}
static inline bool el_strbuf_eql_to_buf(const ElStringBuf* lhs, const ElStringBuf* rhs) {
    return el_strbuf_eql_to(lhs, el_strbuf_view(rhs));
}

static inline bool el_strbuf_starts_with(const ElStringBuf* sb, ElStringView prefix) {
    return el_sv_starts_with(el_strbuf_view(sb), prefix);
}
static inline bool el_strbuf_ends_with(const ElStringBuf* sb, ElStringView suffix) {
    return el_sv_ends_with(el_strbuf_view(sb), suffix);
}
