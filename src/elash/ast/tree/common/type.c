#include <elash/ast/tree/common/type.h>

ElAstTypeNode el_ast_type_name(ElSourceSpan span, ElAstIdentNode* name) {
    return (ElAstTypeNode) {
        .kind = EL_AST_TYPE_NAME,
        .span = span,
        .name = name,
    };
}

ElAstTypeNode el_ast_type_ptr(ElSourceSpan span, ElAstTypeNode* base) {
    return (ElAstTypeNode) {
        .kind = EL_AST_TYPE_PTR,
        .span = span,
        .base = base,
    };
}

ElAstTypeNode* el_ast_new_type_name(ElDynArena* arena, ElSourceSpan span, ElAstIdentNode* name) {
    ElAstTypeNode* node = EL_DYNARENA_NEW(arena, ElAstTypeNode);
    *node = el_ast_type_name(span, name);
    return node;
}

ElAstTypeNode* el_ast_new_type_ptr(ElDynArena* arena, ElSourceSpan span, ElAstTypeNode* base) {
    ElAstTypeNode* node = EL_DYNARENA_NEW(arena, ElAstTypeNode);
    *node = el_ast_type_ptr(span, base);
    return node;
}
