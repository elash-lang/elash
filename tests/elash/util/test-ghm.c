#include <criterion/criterion.h>

#include <elash/util/alloc.h>
#include <elash/util/ghm.h>
#include <elash/util/hash.h>
#include <elash/defs/sv.h>

#include <string.h>

static bool seql(const void* a, const void* b) {
    return strcmp(a, b) == 0;
}
static uhash shash(const void* a) {
    return el_hash_string(el_sv_from_cstr(a));
}

static char* strclone(const char* s) {
    size_t len = strlen(s) + 1;
    char* new = el_alloc(len, 1);
    if (new != NULL) {
        memcpy(new, s, len);
    }
    return new;
}

Test(ghm, basic) {
    ElGHM ghm;
    el_ghm_init(&ghm, shash, seql);

    const char* k1 = "key1";
    const char* v1 = "val1";
    const char* k2 = "key2";
    const char* v2 = "val2";

    el_ghm_insert(&ghm, k1, (void*)v1);
    el_ghm_insert(&ghm, k2, (void*)v2);

    cr_assert_str_eq(el_ghm_lookup(&ghm, k1), v1);
    cr_assert_str_eq(el_ghm_lookup(&ghm, k2), v2);

    cr_assert(el_ghm_remove(&ghm, k1));
    cr_assert_null(el_ghm_lookup(&ghm, k1));
    cr_assert_str_eq(el_ghm_lookup(&ghm, k2), v2);

    el_ghm_free(&ghm);
}

// NOLINTBEGIN(readability-magic-numbers): idfc
Test(ghm, resize) {
    ElGHM ghm;
    el_ghm_init(&ghm, shash, seql);

    char buf[10];
    for (uint i = 0; i < 32; i++) {
        sprintf(buf, "k%d", i);
        char* key = strclone(buf);
        char* val = strclone(buf);
        el_ghm_insert(&ghm, key, val);
    }

    for (uint i = 0; i < 32; i++) {
        sprintf(buf, "k%d", i);
        cr_assert_str_eq(el_ghm_lookup(&ghm, buf), buf);
    }

    el_ghm_free(&ghm);
}
// NOLINTEND(readability-magic-numbers)
