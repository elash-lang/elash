#pragma once

#include <elash/ast/tree/common/ident.h>
#include <elash/ast/tree/expr.h>
#include <elash/ast/tree/type.h>
#include <elash/srcdoc/span.h>
#include <elash/util/dynarena.h>

typedef struct ElAstTypeOrExpr ElAstTypeOrExpr;

typedef enum ElAstTypeOrExprKind {
    EL_AST_TOE_TYPE,
    EL_AST_TOE_EXPR,
    EL_AST_TOE_UNR_IDENT,
    EL_AST_TOE_UNR_INDEX,
} ElAstTypeOrExprKind;

typedef struct ElAstUnrIndex {
    ElAstTypeOrExpr* base;
    ElAstTypeOrExpr* index;
} ElAstUnrIndex;

struct ElAstTypeOrExpr {
    ElAstTypeOrExprKind kind;
    ElSourceSpan span;
    union {
        ElAstType*      type;
        ElAstExpr*      expr;

        ElAstIdent*   unr_ident;
        ElAstUnrIndex unr_index;
    } as;
};

ElAstTypeOrExpr* el_ast_new_toe_type(ElDynArena* arena, ElAstType* type);
ElAstTypeOrExpr* el_ast_new_toe_expr(ElDynArena* arena, ElAstExpr* expr);
ElAstTypeOrExpr* el_ast_new_toe_unr_ident(ElDynArena* arena, ElSourceSpan span, ElAstIdent* ident);
ElAstTypeOrExpr* el_ast_new_toe_unr_index(ElDynArena* arena, ElSourceSpan span, ElAstTypeOrExpr* base, ElAstTypeOrExpr* index);

ElAstType* el_ast_toe_as_type(ElDynArena* arena, ElAstTypeOrExpr* node);
ElAstExpr* el_ast_toe_as_expr(ElDynArena* arena, ElAstTypeOrExpr* node);
