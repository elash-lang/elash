#include <elash/util/strbuf.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#define EL_STRBUF_DEFAULT_CAP 16

static usize _el_strbuf_next_cap(usize current_cap, usize required_min_cap) {
    usize new_cap = current_cap == 0 ? EL_STRBUF_DEFAULT_CAP : current_cap * 2;
    if (new_cap < required_min_cap) {
        new_cap = required_min_cap;
    }
    return new_cap;
}

bool el_strbuf_init(ElStringBuf* sb) {
    return el_strbuf_init_with_cap(sb, EL_STRBUF_DEFAULT_CAP);
}

bool el_strbuf_init_with_cap(ElStringBuf* sb, usize init_cap) {
    sb->data = malloc(init_cap);
    if (sb->data == NULL) return false;
    sb->len = 0;
    sb->cap = init_cap;
    return true;
}

bool el_strbuf_init_from(ElStringBuf* sb, ElStringView sv) {
    sb->data = malloc(sv.len);
    if (sb->data == NULL) return false;

    memcpy(sb->data, sv.data, sv.len);
    sb->len = sv.len;
    sb->cap = sv.len;
    return true;
}

bool el_strbuf_init_from_cstr(ElStringBuf* sb, const char* cstr) {
    return el_strbuf_init_from(sb, el_sv_from_cstr(cstr));
}

void el_strbuf_destroy(ElStringBuf* sb) {
    free(sb->data);
    sb->data = NULL;
    sb->len = 0;
    sb->cap = 0;
}

bool el_strbuf_copy(const ElStringBuf* src, ElStringBuf* dst) {
    if (src->len == 0) {
        dst->data = NULL;
        dst->len = 0;
        dst->cap = 0;
        return true;
    }
    dst->data = malloc(src->len);
    if (dst->data == NULL) {
        dst->len = 0;
        dst->cap = 0;
        return false;
    }
    memcpy(dst->data, src->data, src->len);
    dst->len = src->len;
    dst->cap = src->len;
    return true;
}

void el_strbuf_move(ElStringBuf* src, ElStringBuf* dst) {
    *dst = *src;
    src->data = NULL;
    src->len = 0;
    src->cap = 0;
}

bool el_strbuf_resize(ElStringBuf* sb, usize new_len) {
    if (new_len > sb->cap) {
        if (!el_strbuf_reserve_exact(sb, new_len)) {
            return false;
        }
    }
    sb->len = new_len;
    return true;
}

bool el_strbuf_reserve(ElStringBuf* sb, usize min_cap) {
    if (min_cap <= sb->cap) {
        return true;
    }

    usize new_cap = _el_strbuf_next_cap(sb->cap, min_cap);
    char* new_data = realloc(sb->data, new_cap);
    if (new_data == NULL) return false;

    sb->data = new_data;
    sb->cap = new_cap;
    return true;
}

bool el_strbuf_reserve_exact(ElStringBuf* sb, usize new_cap) {
    if (new_cap <= sb->cap) {
        return true;
    }

    char* new_data = realloc(sb->data, new_cap);
    if (new_data == NULL) return false;

    sb->data = new_data;
    sb->cap = new_cap;
    return true;
}

void el_strbuf_clear(ElStringBuf* sb) {
    sb->len = 0;
}

bool el_strbuf_append(ElStringBuf* sb, ElStringView sv) {
    if (sv.len == 0) return true;
    if (!el_strbuf_reserve(sb, sb->len + sv.len)) {
        return false;
    }

    if (memcpy(sb->data + sb->len, sv.data, sv.len) == NULL) {
        return false;
    }
    sb->len += sv.len;
    return true;
}

bool el_strbuf_append_char(ElStringBuf* sb, char c) {
    if (!el_strbuf_reserve(sb, sb->len + 1)) {
        return false;
    }
    sb->data[sb->len] = c;
    sb->len += 1;
    return true;
}

bool el_strbuf_appendf(ElStringBuf* sb, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int needed = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    if (needed < 0) return false;
    if (needed == 0) return true;

    if (!el_strbuf_reserve(sb, sb->len + needed + 1)) {
        return false;
    }

    va_start(args, fmt);
    vsnprintf(sb->data + sb->len, needed + 1, fmt, args);
    va_end(args);

    sb->len += needed;
    return true;
}
