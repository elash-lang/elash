#include <elash/ast/tree/unr.h>

#include <elash/ast/tree/expr/bin.h>
#include <elash/sema/bin-op.h>
#include <elash/util/assert.h>

ElAstUnr* el_ast_new_unr_ident(ElDynArena* arena, ElSourceSpan span, ElAstIdent* ident) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstUnr, {
        .kind = EL_AST_UNR_IDENT,
        .span = span,
        .as.ident = ident,
    });
}

ElAstUnr* el_ast_new_unr_index(
    ElDynArena* arena, ElSourceSpan span,
    ElAstUnr* base, ElAstUnr* index, ElAstExpr* index_expr
) {
    EL_ASSERT((index == NULL) != (index_expr == NULL), "exactly one index form");
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstUnr, {
        .kind = EL_AST_UNR_INDEX,
        .span = span,
        .as.index = { base, index, index_expr },
    });
}

ElAstType* el_ast_unr_as_type(ElDynArena* arena, ElAstUnr* node) {
    if (node == NULL) return NULL;

    switch (node->kind) {
    case EL_AST_UNR_IDENT:
        return el_ast_new_type_name(arena, node->span, EL_MUTSPEC_DEFAULT, node->as.ident);
    case EL_AST_UNR_INDEX: {
        ElAstType* base = el_ast_unr_as_type(arena, node->as.index.base);
        if (base == NULL) return NULL;

        ElAstExpr* size = node->as.index.index != NULL
            ? el_ast_unr_as_expr(arena, node->as.index.index)
            : node->as.index.index_expr;
        if (size == NULL) return NULL;

        return el_ast_new_type_array(arena, node->span, EL_MUTSPEC_DEFAULT, base, size);
    }
    }
    EL_UNREACHABLE_ENUM_VAL(ElAstUnrKind, node->kind);
}

ElAstExpr* el_ast_unr_as_expr(ElDynArena* arena, ElAstUnr* node) {
    if (node == NULL) return NULL;

    switch (node->kind) {
    case EL_AST_UNR_IDENT:
        return el_ast_new_ident(arena, node->as.ident->span, node->as.ident->name);
    case EL_AST_UNR_INDEX: {
        ElAstExpr* base = el_ast_unr_as_expr(arena, node->as.index.base);
        if (base == NULL) return NULL;

        ElAstExpr* index = node->as.index.index != NULL
            ? el_ast_unr_as_expr(arena, node->as.index.index)
            : node->as.index.index_expr;
        if (index == NULL) return NULL;

        return el_ast_new_bin_expr(
            arena, node->span,
            EL_BIN_OP_INDEX, base, index
        );
    }
    }
    EL_UNREACHABLE_ENUM_VAL(ElAstUnrKind, node->kind);
}

ElAstInit* el_ast_unr_as_init(ElDynArena* arena, ElAstUnr* node) {
    ElAstExpr* expr = el_ast_unr_as_expr(arena, node);
    if (expr == NULL) return NULL;
    return el_ast_new_init_expr(arena, expr);
}
