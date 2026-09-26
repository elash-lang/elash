#include <elash/unparser/unparser.h>

#include <elash/util/assert.h>

void el_unparse_toe(ElUnparser* unpar, ElAstToE* toe) {
    switch (toe->kind) {
    case EL_AST_TOE_TYPE:
        return el_unparse_type(unpar, toe->as.type);
    case EL_AST_TOE_EXPR:
        return el_unparse_expr(unpar, toe->as.expr);
    case EL_AST_TOE_UNR:
        return el_unparse_unr(unpar, toe->as.unr);
    }
    EL_UNREACHABLE_ENUM_VAL(ElAstToEKind, toe->kind);
}
