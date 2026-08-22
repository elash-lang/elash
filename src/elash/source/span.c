#include <elash/source/span.h>

#include <elash/source/doc.h>

#include <stdlib.h>
#include <string.h>

static void el_srcrange_merge_into(ElSourceRange* dst, ElSourceRange src) {
    if (src.doc == NULL) return;

    if (dst->doc == NULL) {
        *dst = src;
        return;
    }

    if (dst->start.offset > src.start.offset) dst->start = src.start;
    if (dst->end.offset < src.end.offset) dst->end = src.end;
}

static int el_srcrange_cmp(const void* a, const void* b) {
    const ElSourceRange* lhs = a;
    const ElSourceRange* rhs = b;

    if (lhs->doc != rhs->doc) {
        if (lhs->doc < rhs->doc) return -1;
        if (lhs->doc > rhs->doc) return 1;
    }

    if (lhs->start.offset < rhs->start.offset) return -1;
    if (lhs->start.offset > rhs->start.offset) return 1;
    return 0;
}

static void el_srcspan_trim_to_center(ElSourceRange* ranges, uint* count) {
    if (*count <= EL_SRCSPAN_MAX_RANGES) return;

    qsort(ranges, *count, sizeof(ElSourceRange), el_srcrange_cmp);

    const uint keep = EL_SRCSPAN_MAX_RANGES;
    const uint start = (*count - keep) / 2;
    memmove(ranges, ranges + start, keep * sizeof(ElSourceRange));
    *count = keep;
}

static void el_srcspan_add_range(ElSourceSpan* span, ElSourceRange range) {
    if (range.doc == NULL) return;

    for (uint i = 0; i < span->count; i++) {
        if (span->ranges[i].doc == range.doc) {
            el_srcrange_merge_into(&span->ranges[i], range);
            return;
        }
    }

    if (span->count < EL_SRCSPAN_MAX_RANGES) {
        span->ranges[span->count++] = range;
        return;
    }

    ElSourceRange tmp[EL_SRCSPAN_MAX_RANGES + 1];
    memcpy(tmp, span->ranges, span->count * sizeof(ElSourceRange));
    tmp[span->count] = range;

    uint count = span->count + 1;
    el_srcspan_trim_to_center(tmp, &count);
    memcpy(span->ranges, tmp, count * sizeof(ElSourceRange));
    span->count = (uint8_t)count;
}

ElSourceSpan el_srcspan_make(const ElSourceDocument* doc, ElSourceLocation start, ElSourceLocation end) {
    ElSourceSpan span = { .count = doc != NULL ? 1 : 0 };
    if (doc != NULL) {
        span.ranges[0] = (ElSourceRange) { .doc = doc, .start = start, .end = end };
    }
    return span;
}

ElStringView el_srcspan_to_sv(ElSourceSpan span) {
    if (span.count == 0 || span.ranges[0].doc == NULL) return EL_SV_NULL;

    const ElSourceRange* range = &span.ranges[0];
    ElStringView full_content = el_srcdoc_content(range->doc);

    if (range->start.offset > range->end.offset) return EL_SV_NULL;
    if (range->end.offset > full_content.len) return EL_SV_NULL;

    return el_sv_slice(full_content, (usize)range->start.offset, (usize)range->end.offset);
}

ElSourceSpan el_srcspan_merge(ElSourceSpan a, ElSourceSpan b) {
    if (!el_srcspan_is_valid(a)) return b;
    if (!el_srcspan_is_valid(b)) return a;

    ElSourceSpan result = a;
    for (uint i = 0; i < b.count; i++) {
        el_srcspan_add_range(&result, b.ranges[i]);
    }
    return result;
}

bool el_srcspan_is_valid(ElSourceSpan span) {
    return span.count > 0 && span.ranges[0].doc != NULL;
}

bool el_srcspan_is_empty(ElSourceSpan span) {
    return el_srcspan_is_valid(span)
        && span.count == 1
        && span.ranges[0].start.offset == span.ranges[0].end.offset;
}
