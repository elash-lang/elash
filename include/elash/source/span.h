#pragma once

#include <elash/defs/source-loc.h>
#include <elash/defs/sv.h>

typedef struct ElSourceDocument ElSourceDocument;

typedef struct ElSourceRange {
    const ElSourceDocument* doc;
    ElSourceLocation        start;
    ElSourceLocation        end;
} ElSourceRange;

#define EL_SRCSPAN_MAX_RANGES 3

typedef struct ElSourceSpan {
    ElSourceRange ranges[EL_SRCSPAN_MAX_RANGES];
    uint8_t       count;
} ElSourceSpan;

#define EL_SRCSPAN_NULL ((ElSourceSpan){ .count = 0 })

ElSourceSpan el_srcspan_make(const ElSourceDocument* doc, ElSourceLocation start, ElSourceLocation end);

/// Returns a string view of the content covered by the primary range.
ElStringView el_srcspan_to_sv(ElSourceSpan span);

/// Merges two spans into one that covers both.
/// Ranges from the same document are coalesced; others are appended.
ElSourceSpan el_srcspan_merge(ElSourceSpan a, ElSourceSpan b);

/// Returns true if the span is not null and has a document.
bool el_srcspan_is_valid(ElSourceSpan span);

/// Returns true if the span is empty (start == end).
bool el_srcspan_is_empty(ElSourceSpan span);
