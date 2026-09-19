#include <elash/ast/tree/init.h>
#include <elash/ast/tree/init/desig.h>
#include <elash/util/dynarena.h>

ElAstDesignator* el_ast_new_desig_member(ElDynArena* arena, ElSourceSpan span, ElStringView member) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstDesignator, {
        .kind = EL_AST_DESIGNATOR_MEMBER,
        .as.member = member,
        .span = span,
        .next = NULL,
    });
}

ElAstDesignator* el_ast_new_desig_tmember(ElDynArena* arena, ElSourceSpan span, usize tmember) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstDesignator, {
        .kind = EL_AST_DESIGNATOR_TMEMBER,
        .as.tmember = tmember,
        .span = span,
        .next = NULL,
    });
}

ElAstDesignator* el_ast_new_desig_index(ElDynArena* arena, ElSourceSpan span, ElAstExpr* index) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstDesignator, {
        .kind = EL_AST_DESIGNATOR_INDEX,
        .as.index = index,
        .span = span,
        .next = NULL,
    });
}

ElAstDesigInitElem* el_ast_new_desig_init_elem(
    ElDynArena* arena, ElSourceSpan span, ElAstDesignator* head, usize desig_count, ElAstInit* init
) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstDesigInitElem, {
        .head = head,
        .desig_count = desig_count,
        .init = init,
        .span = span,
        .next = NULL,
    });
}

ElAstInit* el_ast_new_desig_init(ElDynArena* arena, ElSourceSpan span, ElAstDesigInitElem* head, usize count) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstInit, {
        .kind = EL_AST_INIT_DESIG,
        .desig = {
            .head = head,
            .count = count,
        },
        .span = span,
        .next = NULL,
    });
}

void el_ast_desig_list_append(ElAstDesignator** head, ElAstDesignator** tail, ElAstDesignator* desig) {
    desig->next = NULL;
    if (*tail != NULL) {
        (*tail)->next = desig;
        *tail = desig;
    } else {
        *head = *tail = desig;
    }
}

void el_ast_desig_init_append(ElAstDesigInitElem** head, ElAstDesigInitElem** tail, ElAstDesigInitElem* elem) {
    elem->next = NULL;
    if (*tail != NULL) {
        (*tail)->next = elem;
        *tail = elem;
    } else {
        *head = *tail = elem;
    }
}
