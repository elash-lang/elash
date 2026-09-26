#include <elash/unparser/unparser.h>

#include <elash/util/assert.h>

void el_unparser_unparse_toi(ElUnparser* unpar, ElAstToI* toi) {
    switch (toi->kind) {
    case EL_AST_TOI_TYPE:
        return el_unparser_unparse_type(unpar, toi->as.type);

    case EL_AST_TOI_INIT:
        return el_unparser_unparse_init(unpar, toi->as.init);

    case EL_AST_TOI_UNR:
        return el_unparser_unparse_unr(unpar, toi->as.unr);
    }
    EL_UNREACHABLE_ENUM_VAL(ElAstToIKind, toi->kind);
}
