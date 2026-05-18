#include <elash/hir/tree/expr/literal.h>
#include <elash/hir/tree/expr.h>

ElHirExpr* el_hir_new_int_literal(ElDynArena* arena, ElType* type, int64_t value) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirExpr, {
        .kind = EL_HIR_EXPR_LITERAL,
        .type = type,
        .as.literal.as.int_ = value,
    });
}

ElHirExpr* el_hir_new_uint_literal(ElDynArena* arena, ElType* type, uint64_t value) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirExpr, {
        .kind = EL_HIR_EXPR_LITERAL,
        .type = type,
        .as.literal.as.uint_ = value,
    });
}

ElHirExpr* el_hir_new_char_literal(ElDynArena* arena, ElType* type, char value) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirExpr, {
        .kind = EL_HIR_EXPR_LITERAL,
        .type = type,
        .as.literal.as.char_ = value,
    });
}

ElHirExpr* el_hir_new_bool_literal(ElDynArena* arena, ElType* type, bool value) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirExpr, {
        .kind = EL_HIR_EXPR_LITERAL,
        .type = type,
        .as.literal.as.bool_ = value,
    });
}
