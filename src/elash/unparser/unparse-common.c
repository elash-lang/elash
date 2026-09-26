#include <elash/unparser/unparser.h>

void _el_unparse_ident(ElUnparser* unpar, ElAstIdent* ident) {
    el_unparser_push_ident(unpar, ident->name);
}

void _el_unparse_block(ElUnparser* unpar, ElAstBlockStmt* block) {
    el_unparser_push_punct(unpar, EL_TT_LBRACE);
    for (ElAstStmt* stmt = block->stmts; stmt != NULL; stmt = stmt->next) {
        el_unparse_stmt(unpar, stmt);
    }
    el_unparser_push_punct(unpar, EL_TT_RBRACE);
}

void _el_unparse_func_sig(ElUnparser* unpar, ElAstFuncSignature* sig) {
    el_unparse_type(unpar, sig->ret_type);
    _el_unparse_ident(unpar, sig->name);
    el_unparser_push_punct(unpar, EL_TT_LPAREN);

    for (ElAstFuncParam* param = sig->params.head; param != NULL; param = param->next) {
        el_unparse_type(unpar, param->type);
        _el_unparse_ident(unpar, param->name);
        if (param->next != NULL) {
            el_unparser_push_punct(unpar, EL_TT_COMMA);
        }
    }

    el_unparser_push_punct(unpar, EL_TT_RPAREN);
}
