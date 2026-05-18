#include <elash/ast/tree/common/type.h>

ElAstType* el_ast_new_type_name(ElDynArena* arena, ElSourceSpan span, ElAstIdent* name) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstType, {
        .kind = EL_AST_TYPE_NAME,
        .span = span,
        .name = name,
    });
}

ElAstType* new_with_base(ElDynArena* arena, ElSourceSpan span, ElAstType* base, ElAstTypeKind kind) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstType, {
        .kind = kind,
        .span = span,
        .base = base,
    });
}

ElAstType* el_ast_new_type_ptr(ElDynArena* arena, ElSourceSpan span, ElAstType* base) {
    return new_with_base(arena, span, base, EL_AST_TYPE_PTR);
}
ElAstType* el_ast_new_type_slice(ElDynArena* arena, ElSourceSpan span, ElAstType* base) {
    return new_with_base(arena, span, base, EL_AST_TYPE_SLICE);
}
ElAstType* el_ast_new_type_raw_slice(ElDynArena* arena, ElSourceSpan span, ElAstType* base) {
    return new_with_base(arena, span, base, EL_AST_TYPE_RAW_SLICE);
}

ElAstType* el_ast_new_type_array(ElDynArena* arena, ElSourceSpan span, ElAstType* base, ElAstExpr* size) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstType, {
        .kind = EL_AST_TYPE_ARRAY,
        .span = span,
        .array.base = base,
        .array.size = size,
    });
}
