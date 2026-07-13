#pragma once

#include <elash/diag/severity.h>
#include <elash/sema/type.h>

typedef enum ElDiagMetaType {
    EL_DIAG_META_STRING,
    EL_DIAG_META_INTEGER,
    EL_DIAG_META_CHARACTER,
    EL_DIAG_META_TYPE,
} ElDiagMetaType;

typedef struct ElDiagMetaEntry {
    const char* key;
    ElDiagMetaType type;
    union {
        ElStringView string;
        int integer;
        char character;
        ElType* type;
    } as;
} ElDiagMetaEntry;

#define EL_DIAG_STRING(KEY, STR) \
    ((ElDiagMetaEntry) { .key = (KEY), .type = EL_DIAG_META_STRING, .as.string = (STR) })

#define EL_DIAG_INT(KEY, INT) \
    ((ElDiagMetaEntry) { .key = (KEY), .type = EL_DIAG_META_INTEGER, .as.integer = (INT) })

#define EL_DIAG_CHAR(KEY, CHAR) \
    ((ElDiagMetaEntry) { .key = (KEY), .type = EL_DIAG_META_CHARACTER, .as.integer = (CHAR) })

#define EL_DIAG_TYPE(KEY, TYPE) \
    ((ElDiagMetaEntry) { .key = (KEY), .type = EL_DIAG_META_TYPE, .as.type = (TYPE) })

typedef struct ElDiagMeta {
    const ElDiagMetaEntry* entries;
    usize count;
} ElDiagMeta;

#define EL_DIAG_META(...) ((ElDiagMeta) {                                                     \
        .entries = (const ElDiagMetaEntry[]) { __VA_ARGS__ },                                 \
        .count = sizeof((const ElDiagMetaEntry[]) { __VA_ARGS__ }) / sizeof(ElDiagMetaEntry), \
    })

