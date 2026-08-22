#include <elash/util/pathview.h>

#if EL_PLATFORM_IS_WINDOWS
#include <ctype.h>
#endif

ElPathView el_pathview_dirname(ElPathView pv) {
    if (pv.len == 0) return pv;

    isize last_sep = -1;
    for (isize i = (isize)pv.len - 1; i >= 0; i--) {
        if (EL_IS_PATH_SEP(pv.data[i])) {
            last_sep = i;
            break;
        }
    }

    if (last_sep == -1) {
#if EL_PLATFORM_IS_WINDOWS
        if (pv.len >= 2 && pv.data[1] == ':') {
            return el_sv_slice(pv, 0, 2);
        }
#endif
        return EL_PV_NULL;
    }

    if (last_sep == 0) {
        return el_sv_slice(pv, 0, 1);
    }

#if EL_PLATFORM_IS_WINDOWS
    if (last_sep == 2 && pv.len >= 2 && pv.data[1] == ':') {
        return el_sv_slice(pv, 0, 3);
    }
#endif

    return el_sv_slice(pv, 0, (usize)last_sep);
}

ElPathView el_pathview_basename(ElPathView pv) {
    if (pv.len == 0) return pv;

    isize last_sep = -1;
    for (isize i = (isize)pv.len - 1; i >= 0; i--) {
        if (EL_IS_PATH_SEP(pv.data[i])) {
            last_sep = i;
            break;
        }
    }

    if (last_sep == -1) {
#if EL_PLATFORM_IS_WINDOWS
        if (pv.len >= 2 && pv.data[1] == ':') {
            return el_sv_slice(pv, 2, pv.len);
        }
#endif
        return pv;
    }

    return el_sv_slice(pv, (usize)last_sep + 1, pv.len);
}

ElPathView el_pathview_ext(ElPathView pv) {
    ElPathView base = el_pathview_basename(pv);
    isize last_dot = -1;
    for (isize i = (isize)base.len - 1; i >= 0; i--) {
        if (base.data[i] == '.') {
            last_dot = i;
            break;
        }
    }

    if (last_dot <= 0) {
        return EL_PV_NULL;
    }

    return el_sv_slice(base, (usize)last_dot, base.len);
}

ElPathView el_pathview_stem(ElPathView pv) {
    ElPathView base = el_pathview_basename(pv);
    ElPathView ext = el_pathview_ext(base);
    return el_sv_slice(base, 0, base.len - ext.len);
}

bool el_pathview_is_absolute(ElPathView pv) {
#if EL_PLATFORM_IS_WINDOWS
    if (pv.len >= 2 && isalpha(pv.data[0]) && pv.data[1] == ':') {
        return pv.len >= 3 && EL_IS_PATH_SEP(pv.data[2]);
    }
    return pv.len >= 2 && EL_IS_PATH_SEP(pv.data[0]) && EL_IS_PATH_SEP(pv.data[1]);
#else
    return pv.len > 0 && pv.data[0] == '/';
#endif
}

bool el_pathview_is_root(ElPathView pv) {
#if EL_PLATFORM_IS_WINDOWS
    if (pv.len == 3 && isalpha(pv.data[0]) && pv.data[1] == ':' && EL_IS_PATH_SEP(pv.data[2])) {
        return true;
    }
    if (pv.len == 1 && EL_IS_PATH_SEP(pv.data[0])) {
        return true;
    }
    return false;
#else
    return pv.len == 1 && pv.data[0] == '/';
#endif
}

bool el_pathview_has_sep_at_end(ElPathView pv) {
    return pv.len > 0 && EL_IS_PATH_SEP(pv.data[pv.len - 1]);
}

