#include <criterion/criterion.h>
#include <elash/util/pathbuf.h>

Test(el_pathbuf, sanitize) {
    ElPathBuf pb;
    el_pathbuf_init(&pb);

    el_pathbuf_clear(&pb);
    el_pathbuf_append(&pb, EL_PV("my:file*.txt"));
    el_pathbuf_sanitize(&pb);
    cr_assert(el_strbuf_eql_to_cstr(&pb, "my-file-.txt"));

    el_pathbuf_clear(&pb);
    el_pathbuf_append(&pb, EL_PV("invalid<chars>|in?path\"quote.txt"));
    el_pathbuf_sanitize(&pb);
    cr_assert(el_strbuf_eql_to_cstr(&pb, "invalid-chars--in-path-quote.txt"));

    el_pathbuf_clear(&pb);
    el_pathbuf_append(&pb, EL_PV("dir:one/file:two.txt"));
    el_pathbuf_sanitize(&pb);
    cr_assert(el_strbuf_eql_to_cstr(&pb, "dir-one/file-two.txt"));

#if EL_PLATFORM_IS_WINDOWS
    el_pathbuf_clear(&pb);
    el_pathbuf_append(&pb, EL_PV("C:\\dir:one\\file:two.txt"));
    el_pathbuf_sanitize(&pb);
    cr_assert(el_strbuf_eql_to_cstr(&pb, "C:\\dir-one\\file-two.txt"));
#endif

    el_pathbuf_clear(&pb);
    char ctrl[] = "a\001b\037c\0";
    el_pathbuf_append(&pb, el_sv_from_cstr(ctrl));
    el_pathbuf_sanitize(&pb);
    cr_assert(el_strbuf_eql_to_cstr(&pb, "a-b-c"));

    el_pathbuf_destroy(&pb);
}

Test(el_pathbuf, pop) {
    ElPathBuf pb;
    el_pathbuf_init_from(&pb, EL_PV("a/b/c"));

    cr_assert(el_pathbuf_pop(&pb));
    cr_assert(el_strbuf_eql_to_cstr(&pb, "a/b"));

    cr_assert(el_pathbuf_pop(&pb));
    cr_assert(el_strbuf_eql_to_cstr(&pb, "a"));

    cr_assert(el_pathbuf_pop(&pb));
    cr_assert(el_strbuf_eql_to_cstr(&pb, ""));

    el_pathbuf_destroy(&pb);
}
