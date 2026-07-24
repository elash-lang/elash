#include <elash/ast/tree/toe.h>

#include <elash/util/assert.h>

ElAstToE* el_ast_new_toe_type(ElDynArena* arena, ElAstType* type) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstToE, {
        .kind = EL_AST_TOE_TYPE,
        .span = type->span,
        .as.type = type,
    });
}

ElAstToE* el_ast_new_toe_expr(ElDynArena* arena, ElAstExpr* expr) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstToE, {
        .kind = EL_AST_TOE_EXPR,
        .span = expr->span,
        .as.expr = expr,
    });
}

ElAstToE* el_ast_new_toe_unr_ident(ElDynArena* arena, ElSourceSpan span, ElAstIdent* ident) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstToE, {
        .kind = EL_AST_TOE_UNR_IDENT,
        .span = span,
        .as.unr_ident = ident,
    });
}

ElAstToE* el_ast_new_toe_unr_index(ElDynArena* arena, ElSourceSpan span, ElAstToE* base, ElAstToE* index) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstToE, {
        .kind = EL_AST_TOE_UNR_INDEX,
        .span = span,
        .as.unr_index = { base, index },
    });
}

ElAstType* el_ast_toe_as_type(ElDynArena* arena, ElAstToE* node) {
    if (node == NULL) return NULL;

    switch (node->kind) {
    case EL_AST_TOE_TYPE:
        return node->as.type;
    case EL_AST_TOE_EXPR:
        return NULL;
    case EL_AST_TOE_UNR_INDEX: {
        ElAstType* base = el_ast_toe_as_type(arena, node->as.unr_index.base);
        ElAstExpr* size = el_ast_toe_as_expr(arena, node->as.unr_index.index);
        if (base == NULL || size == NULL) return NULL;
        return el_ast_new_type_array(arena, node->span, base, size);
    }
    case EL_AST_TOE_UNR_IDENT:
        return el_ast_new_type_name(arena, node->span, node->as.unr_ident);
    }
    EL_UNREACHABLE_ENUM_VAL(ElAstToEKind, node->kind);
}

ElAstExpr* el_ast_toe_as_expr(ElDynArena* arena, ElAstToE* node) {
    if (node == NULL) return NULL;

    switch (node->kind) {
    case EL_AST_TOE_EXPR:
        return node->as.expr;
    case EL_AST_TOE_TYPE:
        return NULL;
    case EL_AST_TOE_UNR_INDEX: {
        ElAstExpr* base = el_ast_toe_as_expr(arena, node->as.unr_index.base);
        ElAstExpr* index = el_ast_toe_as_expr(arena, node->as.unr_index.index);
        if (base == NULL || index == NULL) return NULL;
        return el_ast_new_bin_expr(
            arena, node->span,
            EL_SEMA_BIN_OP_INDEX, base, index
        );
    }
    case EL_AST_TOE_UNR_IDENT:
        return el_ast_new_ident(
            arena,
            node->as.unr_ident->span,
            node->as.unr_ident->name
        );
    }
    EL_UNREACHABLE_ENUM_VAL(ElAstToEKind, node->kind);
}
