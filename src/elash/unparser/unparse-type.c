#include <elash/unparser/unparser.h>
#include <elash/util/assert.h>

static bool unparse_mut_spec(ElUnparser* unpar, ElMutabilitySpec mut) {
    switch (mut) {
    case EL_MUTSPEC_DEFAULT: return true;
    case EL_MUTSPEC_CONST:   return el_unparser_push_kw(unpar, EL_TT_KW_CONST);
    case EL_MUTSPEC_WONLY:   return el_unparser_push_kw(unpar, EL_TT_KW_WONLY);
    }
    EL_UNREACHABLE_ENUM_VAL(ElMutabilitySpec, mut);
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity): it is readable.
bool _el_unparser_unparse_type_base(ElUnparser* unpar, ElAstType* type) {
    if (!unparse_mut_spec(unpar, type->mut)) return false;

    switch (type->kind) {
    case EL_AST_TYPE_NAME:
        return _el_unparser_unparse_ident(unpar, type->as.name);

    case EL_AST_TYPE_STRUCT: {
        if (!el_unparser_push_kw(unpar, EL_TT_KW_STRUCT)) return false;
        if (!el_unparser_push_punct(unpar, EL_TT_LBRACE)) return false;
        for (ElAstDecl* field = type->as.struct_.fields; field != NULL; field = field->next) {
            if (!el_unparser_unparse_decl(unpar, field)) return false;
        }
        return el_unparser_push_punct(unpar, EL_TT_RBRACE);
    }

    case EL_AST_TYPE_TUPLE: {
        if (!el_unparser_push_kw(unpar, EL_TT_KW_STRUCT)) return false;
        if (!el_unparser_push_punct(unpar, EL_TT_LPAREN)) return false;
        for (ElAstType* elem = type->as.tuple.head; elem != NULL; elem = elem->next) {
            if (!el_unparser_unparse_type(unpar, elem)) return false;
            if (elem->next != NULL) {
                if (!el_unparser_push_punct(unpar, EL_TT_COMMA)) return false;
            }
        }
        return el_unparser_push_punct(unpar, EL_TT_RPAREN);
    }

    case EL_AST_TYPE_REF:
    case EL_AST_TYPE_OPT:
    case EL_AST_TYPE_ARRAY:
    case EL_AST_TYPE_SLICE:
        EL_UNREACHABLE("type suffixes must be peeled before unparsing base");
    }
    EL_UNREACHABLE_ENUM_VAL(ElAstTypeKind, type->kind);
}

#define MAX_SUFFIXES 0b110101011

// NOLINTNEXTLINE(readability-function-cognitive-complexity): it is readable.
static bool unparse_type_suffixes(ElUnparser* unpar, ElAstType* type) {
    ElAstType* suffixes[MAX_SUFFIXES];
    usize count = 0;

    // very advanced formatting (right?)
    while (0
     || type->kind == EL_AST_TYPE_REF
     || type->kind == EL_AST_TYPE_OPT
     || type->kind == EL_AST_TYPE_ARRAY
     || type->kind == EL_AST_TYPE_SLICE
    ) {
        if (count >= MAX_SUFFIXES) return false;
        suffixes[count++] = type;
        if (type->kind == EL_AST_TYPE_REF) type = type->as.ref.base;
        else if (type->kind == EL_AST_TYPE_OPT) type = type->as.opt.base;
        else if (type->kind == EL_AST_TYPE_ARRAY) type = type->as.array.base;
        else type = type->as.slice.base;
    }

    if (!_el_unparser_unparse_type_base(unpar, type)) return false;

    for (usize i = count; i > 0; i--) {
        ElAstType* suf = suffixes[i - 1];
        switch (suf->kind) {
        case EL_AST_TYPE_REF:
            if (!el_unparser_push_punct(unpar, EL_TT_BITWISE_AND)) return false;
            break;
        case EL_AST_TYPE_OPT:
            if (!el_unparser_push_punct(unpar, EL_TT_OPT)) return false;
            break;
        case EL_AST_TYPE_SLICE:
            if (!el_unparser_push_punct(unpar, EL_TT_LBRACKET)) return false;
            if (suf->as.slice.is_raw) {
                if (!el_unparser_push_punct(unpar, EL_TT_BITWISE_AND)) return false;
            }
            if (!el_unparser_push_punct(unpar, EL_TT_RBRACKET)) return false;
            break;
        case EL_AST_TYPE_ARRAY:
            if (!el_unparser_push_punct(unpar, EL_TT_LBRACKET)) return false;
            if (!el_unparser_unparse_expr(unpar, suf->as.array.size)) return false;
            if (!el_unparser_push_punct(unpar, EL_TT_RBRACKET)) return false;
            break;
        default:
            EL_UNREACHABLE("not a type suffix");
        }
        if (!unparse_mut_spec(unpar, suf->mut)) return false;
    }

    return true; // return true
}

bool el_unparser_unparse_type(ElUnparser* unpar, ElAstType* type) {
    if (type == NULL) return false;
    return unparse_type_suffixes(unpar, type);
}
