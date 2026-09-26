#include <criterion/criterion.h>

#include <elash/lexer/token.h>
#include <elash/source/doc.h>

static void assert_content(const ElSourceDocument* doc, ElStringView expected) {
    cr_assert(el_sv_eql(el_srcdoc_content(doc), expected));
    cr_assert_eq(el_srcdoc_length(doc), expected.len);
}

#define assert_success(status) \
    cr_assert_eq(status, EL_SRCDOC_ERR_SUCCESS);

Test(el_srcdoc_happy, init_append_clear) {
    ElSourceDocument doc;
    el_srcdoc_init_empty(&doc, EL_SV("test.eu"));
    cr_assert(el_sv_eql(doc.filename, EL_SV("test.eu")));
    cr_assert(!doc.is_system);
    assert_content(&doc, EL_SV(""));

    el_srcdoc_append_str(&doc, EL_SV("hello"));
    el_srcdoc_append_str(&doc, EL_SV(" world"));
    assert_content(&doc, EL_SV("hello world"));

    el_srcdoc_clear(&doc);
    assert_content(&doc, EL_SV(""));
    el_srcdoc_free(&doc);
}

Test(el_srcdoc_happy, init_from_string) {
    ElSourceDocument from_string;
    el_srcdoc_init_from_str(&from_string, EL_SV("source"), EL_SV("a.eu"));

    ElStringBuf buffer;
    el_strbuf_init_from(&buffer, EL_SV("Keep yourself safe"));

    ElSourceDocument copied;
    el_srcdoc_init_from_strbuf(&copied, &buffer, EL_SV("b.eu"));
    assert_content(&copied, EL_SV("Keep yourself safe"));

    cr_assert(el_sv_eql(el_strbuf_view(&buffer), EL_SV("Keep yourself safe")));

    ElSourceDocument moved;
    el_srcdoc_init_from_strbuf_move(&moved, &buffer, EL_SV("c.eu"));
    assert_content(&moved, EL_SV("Keep yourself safe"));
    cr_assert_eq(buffer.len, 0);

    el_srcdoc_free(&moved);
    el_srcdoc_free(&copied);
    el_strbuf_destroy(&buffer);
    el_srcdoc_free(&from_string);
}

Test(el_srcdoc_happy, copy_move_and_concat) {
    ElSourceDocument first, second, copy, moved, concated;
    el_srcdoc_init_from_str(&first, EL_SV("one"), EL_SV("one.eu"));
    el_srcdoc_init_from_str(&second, EL_SV("two"), EL_SV("two.eu"));

    el_srcdoc_copy(&first, &copy);
    cr_assert(el_sv_eql(copy.filename, EL_SV("one.eu")));
    el_srcdoc_append_str(&first, EL_SV("!"));
    assert_content(&copy, EL_SV("one"));

    el_srcdoc_move(&copy, &moved);
    assert_content(&moved, EL_SV("one"));
    cr_assert_eq(copy.content.len, 0);

    el_srcdoc_concat(&first, &second, &concated, EL_SV("joined.eu"));
    cr_assert(el_sv_eql(concated.filename, EL_SV("joined.eu")));
    assert_content(&concated, EL_SV("one!two"));

    el_srcdoc_free(&concated); el_srcdoc_free(&moved); el_srcdoc_free(&copy);
    el_srcdoc_free(&second); el_srcdoc_free(&first);
}

Test(el_srcdoc_happy, append_token_raw_forms) {
    ElSourceDocument doc;
    el_srcdoc_init_empty(&doc, EL_SV("tokens.eu"));

    ElToken ident   = { .type = EL_TT_IDENT,          .lexeme = EL_SV("name")     };
    ElToken string  = { .type = EL_TT_STRING_LITERAL, .lexeme = EL_SV("text")     };
    ElToken comment = { .type = EL_TT_LINE_COMMENT,   .lexeme = EL_SV(" comment") };
    ElToken newline = { .type = EL_TT_NEWLINE,        .lexeme = EL_SV("ignored")  };

    el_srcdoc_append_token(&doc, &ident);
    el_srcdoc_append_token(&doc, &string);
    el_srcdoc_append_token(&doc, &comment);
    el_srcdoc_append_token(&doc, &newline);
    assert_content(&doc, EL_SV("name \"text\"// comment\n"));

    el_srcdoc_free(&doc);
}
