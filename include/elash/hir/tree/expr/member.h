#pragma once

#include <elash/hir/type.h>
#include <elash/util/dynarena.h>

typedef struct ElHirExpr ElHirExpr;

typedef struct ElHirMemberExpr {
    ElHirExpr* expr;
    ElStringView name;
} ElHirMemberExpr;

typedef struct ElHirTMemberExpr {
    ElHirExpr* expr;
    usize index;
} ElHirTMemberExpr;

ElHirExpr* el_hir_new_member_expr(ElDynArena* arena, ElHirType* type, ElHirExpr* expr, ElStringView name);
ElHirExpr* el_hir_new_tmember_expr(ElDynArena* arena, ElHirType* type, ElHirExpr* expr, usize index);
