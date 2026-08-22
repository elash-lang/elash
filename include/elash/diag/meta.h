#pragma once

#include <elash/diag/severity.h>
#include <elash/lexer/token.h>
#include <elash/hir/type.h>

typedef enum ElDiagMetaType {
    EL_DIAG_META_TOK,
    EL_DIAG_META_STR,
    EL_DIAG_META_INT,
    EL_DIAG_META_CHAR,
    EL_DIAG_META_TYPE,
} ElDiagMetaType;

typedef struct ElDiagMetaEntry {
    const char* key;
    ElDiagMetaType type;
    union {
        ElStringView string;
        int integer;
        char character;
        ElHirType* type;
        ElToken token;
    } as;
} ElDiagMetaEntry;

#define EL_DIAG_STRING(KEY, STR) \
    ((ElDiagMetaEntry) { .key = (KEY), .type = EL_DIAG_META_STR, .as.string = (STR) })

#define EL_DIAG_INT(KEY, INT) \
    ((ElDiagMetaEntry) { .key = (KEY), .type = EL_DIAG_META_INT, .as.integer = (INT) })

#define EL_DIAG_CHAR(KEY, CHAR) \
    ((ElDiagMetaEntry) { .key = (KEY), .type = EL_DIAG_META_CHAR, .as.character = (CHAR) })

#define EL_DIAG_TYPE(KEY, TYPE) \
    ((ElDiagMetaEntry) { .key = (KEY), .type = EL_DIAG_META_TYPE, .as.type = (TYPE) })

#define EL_DIAG_TOKEN(KEY, TOK) \
    ((ElDiagMetaEntry) { .key = (KEY), .type = EL_DIAG_META_TOK, .as.token = (TOK) })

typedef struct ElDiagMeta {
    const ElDiagMetaEntry* entries;
    usize count;
} ElDiagMeta;

#define EL_DIAG_META(...) ((ElDiagMeta) {                                                     \
        .entries = (const ElDiagMetaEntry[]) { __VA_ARGS__ },                                 \
        .count = sizeof((const ElDiagMetaEntry[]) { __VA_ARGS__ }) / sizeof(ElDiagMetaEntry), \
    })
