#pragma once

#include <elash/util/dynarena.h>
#include <elash/srcdoc/span.h>
#include <elash/ast/tree/common/ident.h>
#include <elash/defs/int-types.h>

typedef struct ElAstExpr ElAstExpr;

typedef struct ElAstMemberExpr {
    ElAstExpr* expr;
    ElStringView name;
} ElAstMemberExpr;

typedef struct ElAstTMemberExpr {
    ElAstExpr* expr;
    usize index;
    ElSourceSpan index_span;
} ElAstTMemberExpr;

ElAstExpr* el_ast_new_member_expr(ElDynArena* arena, ElSourceSpan span, ElAstExpr* expr, ElStringView name);
ElAstExpr* el_ast_new_tmember_expr(ElDynArena* arena, ElSourceSpan span, ElAstExpr* expr, usize index, ElSourceSpan index_span);
