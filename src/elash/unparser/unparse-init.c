#include <elash/unparser/unparser.h>

#include <elash/util/assert.h>

static void unparse_designator(ElUnparser* unpar, ElAstDesignator* desig) {
    switch (desig->kind) {
    case EL_AST_DESIGNATOR_MEMBER:
        el_unparser_push_punct(unpar, EL_TT_DOT);
        el_unparser_push_ident(unpar, desig->as.member);
        return;

    case EL_AST_DESIGNATOR_TMEMBER:
        el_unparser_push_punct(unpar, EL_TT_DOT);
        el_unparser_push_fmt(unpar, EL_TT_INT_LITERAL, "%zu", desig->as.tmember);
        return;

    case EL_AST_DESIGNATOR_INDEX:
        el_unparser_push_punct(unpar, EL_TT_LBRACKET);
        el_unparser_unparse_expr(unpar, desig->as.index);
        el_unparser_push_punct(unpar, EL_TT_RBRACKET);
        return;
    }
    EL_UNREACHABLE_ENUM_VAL(ElAstDesignatorKind, desig->kind);
}

static void unparse_desig_elem(ElUnparser* unpar, ElAstDesigInitElem* elem) {
    for (ElAstDesignator* d = elem->head; d != NULL; d = d->next) {
        unparse_designator(unpar, d);
    }
    el_unparser_push_punct(unpar, EL_TT_ASSIGN);
    return el_unparser_unparse_init(unpar, elem->init);
}


// NOLINTNEXTLINE(readability-function-cognitive-complexity): clang-tidy is broken
void el_unparser_unparse_init(ElUnparser* unpar, ElAstInit* init) {
    switch (init->kind) {
    case EL_AST_INIT_EXPR:
        el_unparser_unparse_expr(unpar, init->expr);
        return;

    case EL_AST_INIT_EMPTY:
        el_unparser_push_punct(unpar, EL_TT_LBRACE);
        el_unparser_push_punct(unpar, EL_TT_RBRACE);
        return;

    case EL_AST_INIT_LIST:
        el_unparser_push_punct(unpar, EL_TT_LBRACE);
        for (ElAstInit* elem = init->list.head; elem != NULL; elem = elem->next) {
            el_unparser_unparse_init(unpar, elem);
            if (elem->next != NULL) {
                el_unparser_push_punct(unpar, EL_TT_COMMA);
            }
        }
        el_unparser_push_punct(unpar, EL_TT_RBRACE);
        return;

    case EL_AST_INIT_DESIG:
        el_unparser_push_punct(unpar, EL_TT_LBRACE);
        for (ElAstDesigInitElem* elem = init->desig.head; elem != NULL; elem = elem->next) {
            unparse_desig_elem(unpar, elem);
            if (elem->next != NULL) {
                el_unparser_push_punct(unpar, EL_TT_COMMA);
            }
        }
        el_unparser_push_punct(unpar, EL_TT_RBRACE);
        return;
    }
    EL_UNREACHABLE_ENUM_VAL(ElAstInitKind, init->kind);
}
