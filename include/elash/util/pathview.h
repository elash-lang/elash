#pragma once

#include <elash/defs/sv.h>
#include <elash/defs/platform.h>

typedef ElStringView ElPathView;

#define EL_PV_NULL EL_SV_NULL
#define EL_PV(LIT) EL_SV(LIT)

#if EL_PLATFORM_IS_WINDOWS
#  define EL_PATH_SEP '\\'
#  define EL_PATH_SEP_STR "\\"
#  define EL_IS_PATH_SEP(c) ((c) == '/' || (c) == '\\')
#else
#  define EL_PATH_SEP '/'
#  define EL_PATH_SEP_STR "/"
#  define EL_IS_PATH_SEP(c) ((c) == '/')
#endif

static inline ElPathView el_pathview_from_cstr(const char* cstr) {
    return el_sv_from_cstr(cstr);
}

ElPathView el_pathview_dirname(ElPathView pv);
ElPathView el_pathview_basename(ElPathView pv);
ElPathView el_pathview_stem(ElPathView pv);
ElPathView el_pathview_ext(ElPathView pv);

bool el_pathview_is_absolute(ElPathView pv);
bool el_pathview_is_root(ElPathView pv);
bool el_pathview_has_sep_at_end(ElPathView pv);

static inline bool el_pathview_is_relative(ElPathView pv) {
    return !el_pathview_is_absolute(pv);
}

