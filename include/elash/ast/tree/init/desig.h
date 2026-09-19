#pragma once

#include <elash/source/span.h>
#include <elash/util/dynarena.h>
#include <elash/defs/int-types.h>

typedef struct ElAstInit ElAstInit;
typedef struct ElAstExpr ElAstExpr;

typedef enum ElAstDesignatorKind {
    EL_AST_DESIGNATOR_MEMBER,   // .foo = ...
    EL_AST_DESIGNATOR_TMEMBER,  // .0 = ...
    EL_AST_DESIGNATOR_INDEX,    // [1] = ...
} ElAstDesignatorKind;

typedef struct ElAstDesignator ElAstDesignator;
struct ElAstDesignator {
    ElAstDesignatorKind kind;
    ElSourceSpan span;
    union {
        ElStringView member;
        usize        tmember;
        ElAstExpr*   index;
    } as;
    ElAstDesignator* next;
};

typedef struct ElAstDesigInitElem ElAstDesigInitElem;
struct ElAstDesigInitElem {
    ElSourceSpan span;
    ElAstDesignator* head;
    usize desig_count;
    ElAstInit* init;
    ElAstDesigInitElem* next;
};

typedef struct ElAstDesignatedInit {
    ElAstDesigInitElem* head;
    usize              count;
} ElAstDesignatedInit;

void el_ast_desig_list_append(ElAstDesignator** head, ElAstDesignator** tail, ElAstDesignator* desig);

ElAstDesignator* el_ast_new_desig_member(ElDynArena* arena, ElSourceSpan span, ElStringView member);
ElAstDesignator* el_ast_new_desig_tmember(ElDynArena* arena, ElSourceSpan span, usize tmember);
ElAstDesignator* el_ast_new_desig_index(ElDynArena* arena, ElSourceSpan span, ElAstExpr* index);

ElAstDesigInitElem* el_ast_new_desig_init_elem(
    ElDynArena* arena, ElSourceSpan span, ElAstDesignator* head, usize desig_count, ElAstInit* init
);

ElAstInit* el_ast_new_desig_init(ElDynArena* arena, ElSourceSpan span, ElAstDesigInitElem* head, usize count);
void el_ast_desig_init_append(ElAstDesigInitElem** head, ElAstDesigInitElem** tail, ElAstDesigInitElem* elem);
