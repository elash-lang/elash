#include <elash/unparser/unparser.h>

#include <elash/util/assert.h>

static void unparse_var_def(ElUnparser* unpar, ElAstDecl* decl) {
    ElAstVarDef* def = &decl->as.var_def;
    if (def->is_static) {
        el_unparser_push_kw(unpar, EL_TT_KW_STATIC);
    }

    el_unparse_type(unpar, def->type);

    for (ElAstDeclarator* d = def->declarators; d != NULL; d = d->next) {
        if (d != def->declarators) {
            el_unparser_push_punct(unpar, EL_TT_COMMA);
        }

        _el_unparse_ident(unpar, d->name);
        if (d->init != NULL) {
            el_unparser_push_punct(unpar, EL_TT_ASSIGN);
            el_unparse_init(unpar, d->init);
        }
    }
    el_unparser_push_punct(unpar, EL_TT_SEMICOLON);
}

static void unparse_var_decl(ElUnparser* unpar, ElAstDecl* decl) {
    ElAstVarDecl* vd = &decl->as.var_decl;
    el_unparser_push_kw(unpar, EL_TT_KW_EXTERN);

    el_unparse_type(unpar, vd->type);
    for (ElAstDeclarator* d = vd->declarators; d != NULL; d = d->next) {
        if (d != vd->declarators) {
            el_unparser_push_punct(unpar, EL_TT_COMMA);
        }
        _el_unparse_ident(unpar, d->name);
    }
    el_unparser_push_punct(unpar, EL_TT_SEMICOLON);
}

static void unparse_func_def(ElUnparser* unpar, ElAstDecl* decl) {
    _el_unparse_func_sig(unpar, &decl->as.func_def.sig);
    _el_unparse_block(unpar, decl->as.func_def.block);
}

static void unparse_func_decl(ElUnparser* unpar, ElAstDecl* decl) {
    _el_unparse_func_sig(unpar, &decl->as.func_decl.sig);
    el_unparser_push_punct(unpar, EL_TT_SEMICOLON);
}

static void unparse_alias(ElUnparser* unpar, ElAstDecl* decl) {
    el_unparser_push_kw(unpar, EL_TT_KW_ALIAS);
    el_unparser_push_ident(unpar, decl->as.alias.name);
    el_unparser_push_punct(unpar, EL_TT_ASSIGN);
    el_unparse_toe(unpar, &decl->as.alias.target);
    el_unparser_push_punct(unpar, EL_TT_SEMICOLON);
}

static void unparse_typedef(ElUnparser* unpar, ElAstDecl* decl) {
    el_unparser_push_kw(unpar, EL_TT_KW_TYPEDEF);
    el_unparser_push_ident(unpar, decl->as.typedef_.name);
    if (decl->as.typedef_.target != NULL) {
        el_unparser_push_kw(unpar, EL_TT_KW_AS);
        el_unparse_type(unpar, decl->as.typedef_.target);
    }
    el_unparser_push_punct(unpar, EL_TT_SEMICOLON);
}

void el_unparse_decl(ElUnparser* unpar, ElAstDecl* decl) {
    switch (decl->type) {
    case EL_AST_DECL_VAR_DEF:   return unparse_var_def(unpar, decl);
    case EL_AST_DECL_VAR_DECL:  return unparse_var_decl(unpar, decl);
    case EL_AST_DECL_FUNC_DEF:  return unparse_func_def(unpar, decl);
    case EL_AST_DECL_FUNC_DECL: return unparse_func_decl(unpar, decl);
    case EL_AST_DECL_ALIAS:     return unparse_alias(unpar, decl);
    case EL_AST_DECL_TYPEDEF:   return unparse_typedef(unpar, decl);
    }
    EL_UNREACHABLE_ENUM_VAL(ElAstDeclType, decl->type);
}
