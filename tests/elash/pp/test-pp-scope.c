#include <criterion/criterion.h>

#include <elash/pp/symbol.h>
#include <elash/pp/scope.h>
#include <elash/defs/sv.h>

static ElDynArena arena;
void init() { el_dynarena_init(&arena); }
void fini() { el_dynarena_free(&arena); }

TestSuite(pp_scope, .init = init, .fini = fini);

// NOLINTBEGIN(readability-magic-numbers)

Test(pp_scope, basic_usage) {
    ElPpScope* scope = el_pp_scope_new(NULL);

    ElPpSymbol var1 = {
        .name = EL_SV("foo"),
        .as.var.v = EL_DYNARENA_NEW_STRUCT(&arena, ElPpValue, {
            .type = EL_PP_TYPE_INT,
            .as.int_ = 42,
        }),
    };

    ElStringView key = EL_SV("foo");
    cr_assert(el_pp_scope_assign(scope, key, &var1));

    ElPpSymbol* lookup_result = el_pp_scope_lookup(scope, key);
    cr_assert_not_null(lookup_result);
    cr_assert_eq(lookup_result, &var1);
    cr_assert_eq(lookup_result->as.var.v->as.int_, 42);

    cr_assert(el_pp_scope_deassign(scope, key));
    cr_assert_null(el_pp_scope_lookup(scope, key));

    el_pp_scope_free(scope);
}

Test(pp_scope, nested_lookup) {
    ElPpScope* parent = el_pp_scope_new(NULL);
    ElPpScope* child = el_pp_scope_new(parent);

    ElPpSymbol var_parent = {
        .name = EL_SV("foo"),
        .as.var.v = EL_DYNARENA_NEW_STRUCT(&arena, ElPpValue, {
            .type = EL_PP_TYPE_INT,
            .as.int_ = 100,
        }),
    };

    ElStringView key = EL_SV("foo");
    el_pp_scope_assign(parent, key, &var_parent);

    ElPpSymbol* lookup = el_pp_scope_lookup(child, key);
    cr_assert_not_null(lookup);
    cr_assert_eq(lookup, &var_parent, "child should see parent variable");

    el_pp_scope_free(child);
    el_pp_scope_free(parent);
}

Test(pp_scope, shadowing) {
    ElPpScope* parent = el_pp_scope_new(NULL);
    ElPpScope* child = el_pp_scope_new(parent);

    ElPpSymbol var_parent = {
        .name = EL_SV("foo"),
        .as.var.v = EL_DYNARENA_NEW_STRUCT(&arena, ElPpValue, {
            .type = EL_PP_TYPE_FLOAT,
            .as.float_ = 3.14f,
        }),
    };

    ElPpSymbol var_child = {
        .name = EL_SV("foo"),
        .as.var.v = EL_DYNARENA_NEW_STRUCT(&arena, ElPpValue, {
            .type = EL_PP_TYPE_FLOAT,
            .as.float_ = 2.19f,
        }),
    };

    ElStringView key = EL_SV("foo");
    el_pp_scope_assign(parent, key, &var_parent);
    el_pp_scope_assign(child, key, &var_child);

    ElPpSymbol* lookup_child = el_pp_scope_lookup(child, key);
    cr_assert_eq(lookup_child, &var_child, "child should shadow parent");

    ElPpSymbol* lookup_parent = el_pp_scope_lookup(parent, key);
    cr_assert_eq(lookup_parent, &var_parent, "parent should remain unaffected by shadowing");

    el_pp_scope_free(child);
    el_pp_scope_free(parent);
}

// NOLINTEND(readability-magic-numbers)
