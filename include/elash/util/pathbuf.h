#pragma once

#include <elash/util/pathview.h>
#include <elash/util/strbuf.h>

typedef ElStringBuf ElPathBuf;

static inline bool el_pathbuf_init(ElPathBuf* pb)                     { return el_strbuf_init(pb);          }
static inline bool el_pathbuf_init_from(ElPathBuf* pb, ElPathView pv) { return el_strbuf_init_from(pb, pv); }
static inline ElPathView el_pathbuf_view(const ElPathBuf* pb)         { return el_strbuf_view(pb);          }

static inline void el_pathbuf_destroy(ElPathBuf* pb) { el_strbuf_destroy(pb); }
static inline void el_pathbuf_clear(ElPathBuf* pb)   { el_strbuf_clear(pb);   }

bool el_pathbuf_pop(ElPathBuf* pb);
bool el_pathbuf_join(ElPathBuf* pb, ElPathView pv);
bool el_pathbuf_append(ElPathBuf* pb, ElPathView pv);
bool el_pathbuf_set_ext(ElPathBuf* pb, ElPathView ext);

void el_pathbuf_sanitize(ElPathBuf* pb);
