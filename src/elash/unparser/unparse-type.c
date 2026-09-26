#include <elash/unparser/unparser.h>
#include <elash/util/assert.h>

static void unparse_mut_spec(ElUnparser* unpar, ElMutabilitySpec mut) {
    switch (mut) {
    case EL_MUTSPEC_DEFAULT: return;
    case EL_MUTSPEC_CONST:   return el_unparser_push_kw(unpar, EL_TT_KW_CONST);
    case EL_MUTSPEC_WONLY:   return el_unparser_push_kw(unpar, EL_TT_KW_WONLY);
    }
    EL_UNREACHABLE_ENUM_VAL(ElMutabilitySpec, mut);
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity): it is readable.
void _el_unparse_type_base(ElUnparser* unpar, ElAstType* type) {
    unparse_mut_spec(unpar, type->mut);

    switch (type->kind) {
    case EL_AST_TYPE_NAME:
        _el_unparse_ident(unpar, type->as.name);
        return;

    case EL_AST_TYPE_STRUCT:
        el_unparser_push_kw(unpar, EL_TT_KW_STRUCT);
        el_unparser_push_punct(unpar, EL_TT_LBRACE);
        for (ElAstDecl* field = type->as.struct_.fields; field != NULL; field = field->next) {
            el_unparse_decl(unpar, field);
        }
        el_unparser_push_punct(unpar, EL_TT_RBRACE);
        return;

    case EL_AST_TYPE_TUPLE:
        el_unparser_push_kw(unpar, EL_TT_KW_STRUCT);
        el_unparser_push_punct(unpar, EL_TT_LPAREN);
        for (ElAstType* elem = type->as.tuple.head; elem != NULL; elem = elem->next) {
            el_unparse_type(unpar, elem);
            if (elem->next != NULL) {
                el_unparser_push_punct(unpar, EL_TT_COMMA);
            }
        }
        el_unparser_push_punct(unpar, EL_TT_RPAREN);
        return;

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
void el_unparse_type(ElUnparser* unpar, ElAstType* type) {
    ElAstType* suffixes[MAX_SUFFIXES];
    usize count = 0;

    // very advanced formatting (right?)
    while (0
     || type->kind == EL_AST_TYPE_REF
     || type->kind == EL_AST_TYPE_OPT
     || type->kind == EL_AST_TYPE_ARRAY
     || type->kind == EL_AST_TYPE_SLICE
    ) {
        EL_ASSERT(count < MAX_SUFFIXES, "too many suffixes");
        suffixes[count++] = type;

        if      (type->kind == EL_AST_TYPE_REF)   type = type->as.ref.base;
        else if (type->kind == EL_AST_TYPE_OPT)   type = type->as.opt.base;
        else if (type->kind == EL_AST_TYPE_ARRAY) type = type->as.array.base;
        else if (type->kind == EL_AST_TYPE_SLICE) type = type->as.slice.base;
        else EL_UNREACHABLE("should not be possible");
    }

    _el_unparse_type_base(unpar, type);

    for (usize i = count; i > 0; i--) {
        ElAstType* suf = suffixes[i - 1];
        switch (suf->kind) {
        case EL_AST_TYPE_REF:
            el_unparser_push_punct(unpar, EL_TT_BITWISE_AND);
            break;
        case EL_AST_TYPE_OPT:
            el_unparser_push_punct(unpar, EL_TT_OPT);
            break;
        case EL_AST_TYPE_SLICE:
            el_unparser_push_punct(unpar, EL_TT_LBRACKET);
            if (suf->as.slice.is_raw) {
                el_unparser_push_punct(unpar, EL_TT_BITWISE_AND);
            }
            el_unparser_push_punct(unpar, EL_TT_RBRACKET);
            break;
        case EL_AST_TYPE_ARRAY:
            el_unparser_push_punct(unpar, EL_TT_LBRACKET);
            el_unparse_expr(unpar, suf->as.array.size);
            el_unparser_push_punct(unpar, EL_TT_RBRACKET);
            break;
        default:
            EL_UNREACHABLE("not a type suffix");
        }
        unparse_mut_spec(unpar, suf->mut);
    }
}
