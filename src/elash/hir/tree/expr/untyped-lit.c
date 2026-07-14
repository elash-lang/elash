#include <elash/hir/tree/expr/untyped-lit.h>
#include <elash/hir/tree/expr.h>

ElHirExpr* el_hir_new_untyped_int_lit(ElDynArena* arena, int64_t value) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirExpr, {
        .kind = EL_HIR_EXPR_UNTYPEDLIT,
        .type = NULL,
        .as.untyped_lit = {
            .kind = EL_HIR_UNTYPED_INT,
            .of.int_ = value,
        },
    });
}

ElHirExpr* el_hir_new_untyped_char_lit(ElDynArena* arena, char value) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirExpr, {
        .kind = EL_HIR_EXPR_UNTYPEDLIT,
        .type = NULL,
        .as.untyped_lit = {
            .kind = EL_HIR_UNTYPED_CHAR,
            .of.char_ = value,
        },
    });
}

ElHirExpr* el_hir_new_untyped_bool_lit(ElDynArena* arena, bool value) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirExpr, {
        .kind = EL_HIR_EXPR_UNTYPEDLIT,
        .type = NULL,
        .as.untyped_lit = {
            .kind = EL_HIR_UNTYPED_BOOL,
            .of.bool_ = value,
        },
    });
}

ElStringView el_hir_untyped_lit_kind_to_string(ElHirUntypedLitKind lit) {
    switch (lit) {
    case EL_HIR_UNTYPED_INT:  return EL_SV("integer");
    case EL_HIR_UNTYPED_CHAR: return EL_SV("character");
    case EL_HIR_UNTYPED_BOOL: return EL_SV("boolean");
    }
}
