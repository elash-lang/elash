#pragma once

#include <elash/source/span.h>
#include <elash/defs/sv.h>

typedef struct ElPpIncMapping ElPpIncMapping;

// include mapping represents a scope-to-path mapping
struct ElPpIncMapping {
    bool is_system;
    ElStringView name;
    ElStringView path;
    ElPpIncMapping* next;
};

// include map represents a list of mappings
typedef struct ElPpIncMap {
    ElPpIncMapping* head;
    ElPpIncMapping* tail;
} ElPpIncMap;

// include path represents a #include/#embed directive param
// consists of the scope and the path
typedef struct ElPpIncPath {
    ElStringView scope;
    ElStringView ipath;
    ElSourceSpan  span;
} ElPpIncPath;

// include file represents a resolved include path
typedef struct ElPpIncFile {
    bool is_system;
    ElStringView path;
} ElPpIncFile;

static inline void el_pp_imap_add(ElPpIncMap* paths, ElPpIncMapping* mapping) {
    mapping->next = NULL;
    if (paths->head == NULL) {
        paths->head = mapping;
        paths->tail = mapping;
    } else {
        paths->tail->next = mapping;
        paths->tail = mapping;
    }
}
