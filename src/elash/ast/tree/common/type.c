#include <elash/ast/tree/common/type.h>

ElAstTypeNode* el_ast_new_type_name(ElDynArena* arena, ElSourceSpan span, ElAstIdentNode* name) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstTypeNode, {
        .kind = EL_AST_TYPE_NAME,
        .span = span,
        .name = name,
    });
}

ElAstTypeNode* new_with_base(ElDynArena* arena, ElSourceSpan span, ElAstTypeNode* base, ElAstTypeKind kind) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstTypeNode, {
        .kind = kind,
        .span = span,
        .base = base,
    });
}

ElAstTypeNode* el_ast_new_type_ptr(ElDynArena* arena, ElSourceSpan span, ElAstTypeNode* base) {
    return new_with_base(arena, span, base, EL_AST_TYPE_PTR);
}
ElAstTypeNode* el_ast_new_type_slice(ElDynArena* arena, ElSourceSpan span, ElAstTypeNode* base) {
    return new_with_base(arena, span, base, EL_AST_TYPE_SLICE);
}
ElAstTypeNode* el_ast_new_type_raw_slice(ElDynArena* arena, ElSourceSpan span, ElAstTypeNode* base) {
    return new_with_base(arena, span, base, EL_AST_TYPE_RAW_SLICE);
}

ElAstTypeNode* el_ast_new_type_array(ElDynArena* arena, ElSourceSpan span, ElAstTypeNode* base, ElAstExprNode* size) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstTypeNode, {
        .kind = EL_AST_TYPE_ARRAY,
        .span = span,
        .array.base = base,
        .array.size = size,
    });
}


