#pragma once

#include <elash/ast/tree/common/ident.h>
#include <elash/ast/tree/expr.h>
#include <elash/ast/tree/type.h>
#include <elash/srcdoc/span.h>
#include <elash/util/dynarena.h>

typedef struct ElAstToE ElAstToE;

typedef enum ElAstToEKind {
    EL_AST_TOE_TYPE,
    EL_AST_TOE_EXPR,
    EL_AST_TOE_UNR_IDENT,
    EL_AST_TOE_UNR_INDEX,
} ElAstToEKind;

typedef struct ElAstUnrIndex {
    ElAstToE* base;
    ElAstToE* index;
} ElAstUnrIndex;

struct ElAstToE {
    ElAstToEKind kind;
    ElSourceSpan span;
    union {
        ElAstType*      type;
        ElAstExpr*      expr;

        ElAstIdent*   unr_ident;
        ElAstUnrIndex unr_index;
    } as;
};

ElAstToE* el_ast_new_toe_type(ElDynArena* arena, ElAstType* type);
ElAstToE* el_ast_new_toe_expr(ElDynArena* arena, ElAstExpr* expr);
ElAstToE* el_ast_new_toe_unr_ident(ElDynArena* arena, ElSourceSpan span, ElAstIdent* ident);
ElAstToE* el_ast_new_toe_unr_index(ElDynArena* arena, ElSourceSpan span, ElAstToE* base, ElAstToE* index);

ElAstType* el_ast_toe_as_type(ElDynArena* arena, ElAstToE* node);
ElAstExpr* el_ast_toe_as_expr(ElDynArena* arena, ElAstToE* node);
