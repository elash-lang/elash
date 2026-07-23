#include <elash/hir/tree/expr/const.h>
#include <elash/hir/tree/expr.h>

ElHirExpr* el_hir_new_int_constant(ElDynArena* arena, ElSourceSpan span, ElHirType* type, int64_t value) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirExpr, {
        .kind = EL_HIR_EXPR_CONST,
        .type = type,
        .span = span,
        .as.constant.as.int_ = value,
    });
}

ElHirExpr* el_hir_new_char_constant(ElDynArena* arena, ElSourceSpan span, ElHirType* type, char value) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirExpr, {
        .kind = EL_HIR_EXPR_CONST,
        .type = type,
        .span = span,
        .as.constant.as.char_ = value,
    });
}

ElHirExpr* el_hir_new_bool_constant(ElDynArena* arena, ElSourceSpan span, ElHirType* type, bool value) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElHirExpr, {
        .kind = EL_HIR_EXPR_CONST,
        .type = type,
        .span = span,
        .as.constant.as.bool_ = value,
    });
}
