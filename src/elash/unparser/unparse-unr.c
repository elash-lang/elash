#include <elash/unparser/unparser.h>

#include <elash/util/assert.h>

void el_unparser_unparse_unr(ElUnparser* unpar, ElAstUnr* unr) {
    switch (unr->kind) {
    case EL_AST_UNR_IDENT:
        _el_unparser_unparse_ident(unpar, unr->as.ident);
        return;

    case EL_AST_UNR_INDEX:
        el_unparser_unparse_unr(unpar, unr->as.index.base);
        el_unparser_push_punct(unpar, EL_TT_LBRACKET);
        if (unr->as.index.index != NULL) {
            el_unparser_unparse_unr(unpar, unr->as.index.index);
        } else {
            el_unparser_unparse_expr(unpar, unr->as.index.index_expr);
        }
        el_unparser_push_punct(unpar, EL_TT_RBRACKET);
        return;
    }
    EL_UNREACHABLE_ENUM_VAL(ElAstUnrKind, unr->kind);
}
