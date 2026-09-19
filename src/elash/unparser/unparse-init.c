#include <elash/unparser/unparser.h>

#include <elash/util/assert.h>

static bool unparse_designator(ElUnparser* unpar, ElAstDesignator* desig) {
    switch (desig->kind) {
    case EL_AST_DESIGNATOR_MEMBER:
        if (!el_unparser_push_punct(unpar, EL_TT_DOT)) return false;
        return el_unparser_push_ident(unpar, desig->as.member);

    case EL_AST_DESIGNATOR_TMEMBER:
        if (!el_unparser_push_punct(unpar, EL_TT_DOT)) return false;
        return el_unparser_push_fmt(unpar, EL_TT_INT_LITERAL, "%zu", desig->as.tmember);

    case EL_AST_DESIGNATOR_INDEX:
        if (!el_unparser_push_punct(unpar, EL_TT_LBRACKET))    return false;
        if (!el_unparser_unparse_expr(unpar, desig->as.index)) return false;
        return el_unparser_push_punct(unpar, EL_TT_RBRACKET);
    }
    EL_UNREACHABLE_ENUM_VAL(ElAstDesignatorKind, desig->kind);
}

static bool unparse_desig_elem(ElUnparser* unpar, ElAstDesigInitElem* elem) {
    for (ElAstDesignator* d = elem->head; d != NULL; d = d->next) {
        if (!unparse_designator(unpar, d)) return false;
    }
    if (!el_unparser_push_punct(unpar, EL_TT_ASSIGN)) return false;
    return el_unparser_unparse_init(unpar, elem->init);
}


// NOLINTNEXTLINE(readability-function-cognitive-complexity): clang-tidy is broken
bool el_unparser_unparse_init(ElUnparser* unpar, ElAstInit* init) {
    switch (init->kind) {
    case EL_AST_INIT_EXPR:
        return el_unparser_unparse_expr(unpar, init->expr);

    case EL_AST_INIT_EMPTY:
        if (!el_unparser_push_punct(unpar, EL_TT_LBRACE)) return false;
        return el_unparser_push_punct(unpar, EL_TT_RBRACE);

    case EL_AST_INIT_LIST:
        if (!el_unparser_push_punct(unpar, EL_TT_LBRACE)) return false;
        for (ElAstInit* elem = init->list.head; elem != NULL; elem = elem->next) {
            if (!el_unparser_unparse_init(unpar, elem)) return false;
            if (elem->next != NULL) {
                if (!el_unparser_push_punct(unpar, EL_TT_COMMA)) return false;
            }
        }
        return el_unparser_push_punct(unpar, EL_TT_RBRACE);

    case EL_AST_INIT_DESIG:
        if (!el_unparser_push_punct(unpar, EL_TT_LBRACE)) return false;
        for (ElAstDesigInitElem* elem = init->desig.head; elem != NULL; elem = elem->next) {
            if (!unparse_desig_elem(unpar, elem)) return false;
            if (elem->next != NULL) {
                if (!el_unparser_push_punct(unpar, EL_TT_COMMA)) return false;
            }
        }
        return el_unparser_push_punct(unpar, EL_TT_RBRACE);
    }
    EL_UNREACHABLE_ENUM_VAL(ElAstInitKind, init->kind);
}
