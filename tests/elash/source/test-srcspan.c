#include <criterion/criterion.h>

#include <elash/source/doc.h>
#include <elash/source/span.h>

// NOLINTBEGIN(readability-magic-numbers)
// (the entire test is made by ai im too lazy to format it so it looks
//  like shit but who cares at least it tests something (i think))

static ElSourceLocation loc(uint offset) {
    return (ElSourceLocation) { .line = 1, .column = offset + 1, .offset = offset };
}

static void assert_span_text(ElSourceSpan span, ElStringView expected) {
    cr_assert(el_sv_eql(el_srcspan_to_sv(span), expected));
}

TestSuite(el_srcspan_happy);

Test(el_srcspan_happy, make_and_query) {
    ElSourceDocument doc;
    cr_assert_eq(el_srcdoc_init_from_str(&doc, EL_SV("abcdef"), EL_SV("test.eu")),
                 EL_SRCDOC_ERR_SUCCESS);

    ElSourceSpan span = el_srcspan_make(&doc, loc(1), loc(4));
    cr_assert(el_srcspan_is_valid(span));
    cr_assert(!el_srcspan_is_empty(span));
    assert_span_text(span, EL_SV("bcd"));

    ElSourceSpan empty = el_srcspan_make(&doc, loc(3), loc(3));
    cr_assert(el_srcspan_is_valid(empty));
    cr_assert(el_srcspan_is_empty(empty));
    assert_span_text(empty, EL_SV(""));

    cr_assert(!el_srcspan_is_valid(EL_SRCSPAN_NULL));
    cr_assert(!el_srcspan_is_empty(EL_SRCSPAN_NULL));
    el_srcdoc_free(&doc);
}

Test(el_srcspan_happy, merge_ranges_in_same_document) {
    ElSourceDocument doc;
    cr_assert_eq(el_srcdoc_init_from_str(&doc, EL_SV("0123456789"), EL_SV("test.eu")),
                 EL_SRCDOC_ERR_SUCCESS);

    ElSourceSpan left = el_srcspan_make(&doc, loc(1), loc(3));
    ElSourceSpan right = el_srcspan_make(&doc, loc(6), loc(9));
    ElSourceSpan merged = el_srcspan_merge(left, right);
    cr_assert_eq(merged.count, 1);
    cr_assert_eq(merged.ranges[0].start.offset, 1);
    cr_assert_eq(merged.ranges[0].end.offset, 9);
    assert_span_text(merged, EL_SV("12345678"));

    ElSourceSpan reverse = el_srcspan_merge(right, left);
    cr_assert_eq(reverse.count, 1);
    cr_assert_eq(reverse.ranges[0].start.offset, 1);
    cr_assert_eq(reverse.ranges[0].end.offset, 9);
    el_srcdoc_free(&doc);
}

Test(el_srcspan_happy, merge_different_documents_and_null) {
    ElSourceDocument first, second;
    cr_assert_eq(el_srcdoc_init_from_str(&first, EL_SV("first"), EL_SV("one.eu")),
                 EL_SRCDOC_ERR_SUCCESS);
    cr_assert_eq(el_srcdoc_init_from_str(&second, EL_SV("second"), EL_SV("two.eu")),
                 EL_SRCDOC_ERR_SUCCESS);

    ElSourceSpan a = el_srcspan_make(&first, loc(0), loc(2));
    ElSourceSpan b = el_srcspan_make(&second, loc(1), loc(4));
    ElSourceSpan merged = el_srcspan_merge(a, b);
    cr_assert_eq(merged.count, 2);
    cr_assert_eq(merged.ranges[0].doc, &first);
    cr_assert_eq(merged.ranges[1].doc, &second);
    cr_assert(el_srcspan_merge(EL_SRCSPAN_NULL, a).ranges[0].doc == &first);
    cr_assert(el_srcspan_merge(b, EL_SRCSPAN_NULL).ranges[0].doc == &second);

    el_srcdoc_free(&second);
    el_srcdoc_free(&first);
}

Test(el_srcspan_happy, invalid_ranges_have_no_text) {
    ElSourceDocument doc;
    cr_assert_eq(el_srcdoc_init_from_str(&doc, EL_SV("abc"), EL_SV("test.eu")),
                 EL_SRCDOC_ERR_SUCCESS);

    ElSourceSpan backwards = el_srcspan_make(&doc, loc(2), loc(1));
    cr_assert(el_srcspan_is_valid(backwards));
    cr_assert(el_sv_is_null(el_srcspan_to_sv(backwards)));

    ElSourceSpan past_end = el_srcspan_make(&doc, loc(0), loc(4));
    cr_assert(el_sv_is_null(el_srcspan_to_sv(past_end)));
    cr_assert(!el_srcspan_is_empty(past_end));
    el_srcdoc_free(&doc);
}

// NOLINTEND(readability-magic-numbers)
