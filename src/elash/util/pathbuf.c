#include <elash/util/pathbuf.h>

bool el_pathbuf_append(ElPathBuf* pb, ElPathView pv) {
    return el_strbuf_append(pb, pv);
}

bool el_pathbuf_join(ElPathBuf* pb, ElPathView pv) {
    if (pv.len == 0) return true;

    if (pb->len > 0) {
        bool buf_has_sep = EL_IS_PATH_SEP(pb->data[pb->len - 1]);
        bool view_has_sep = EL_IS_PATH_SEP(pv.data[0]);

        if (!buf_has_sep && !view_has_sep) {
            if (!el_strbuf_append_char(pb, EL_PATH_SEP)) {
                return false;
            }
        } else if (buf_has_sep) {
            while (pv.len > 0 && EL_IS_PATH_SEP(pv.data[0])) {
                pv = el_sv_slice(pv, 1, pv.len);
            }
        }
    }

    return el_strbuf_append(pb, pv);
}

bool el_pathbuf_pop(ElPathBuf* pb) {
    ElPathView view = el_pathbuf_view(pb);
    ElPathView dir = el_pathview_dirname(view);

    if (dir.len == 0 && pb->len > 0) {
        if (el_pathview_is_root(view)) {
            return true;
        }
        el_strbuf_clear(pb);
        return true;
    }

    return el_strbuf_resize(pb, dir.len);
}

bool el_pathbuf_set_ext(ElPathBuf* pb, ElPathView ext) {
    ElPathView current_view = el_pathbuf_view(pb);
    ElPathView current_ext = el_pathview_ext(current_view);

    if (current_ext.len > 0) {
        if (!el_strbuf_resize(pb, pb->len - current_ext.len)) {
            return false;
        }
    }

    if (ext.len > 0) {
        if (ext.data[0] != '.') {
            if (!el_strbuf_append_char(pb, '.')) {
                return false;
            }
        }
        return el_strbuf_append(pb, ext);
    }

    return true;
}

#define FIRST_PRINTABLE 32

void el_pathbuf_sanitize(ElPathBuf* pb) {
    usize start = 0;
#if EL_PLATFORM_IS_WINDOWS
    if (pb->len >= 2 && pb->data[1] == ':' &&
        ((pb->data[0] >= 'a' && pb->data[0] <= 'z') || (pb->data[0] >= 'A' && pb->data[0] <= 'Z'))) {
        start = 2;
    }
#endif

    for (usize i = start; i < pb->len; i++) {
        char c = pb->data[i];
        if (EL_IS_PATH_SEP(c)) continue;

        switch (c) {
        case '<':
        case '>':
        case ':':
        case '"':
        case '|':
        case '?':
        case '*':
            pb->data[i] = '-';
            break;
        default:
            if ((unsigned char)c < FIRST_PRINTABLE || (!EL_PLATFORM_IS_WINDOWS && c == '\\')) {
                pb->data[i] = '-';
            }
            break;
        }
    }
}

