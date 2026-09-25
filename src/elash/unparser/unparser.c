#include <elash/unparser/unparser.h>

#include <elash/lexer/token.h>

#include <stdarg.h>
#include <stdio.h>

void el_unparser_init(ElUnparser* unpar, ElTokenBuf* out, ElDynArena* arena) {
    unpar->out = out;
    unpar->arena = arena;
}

bool el_unparser_push(ElUnparser* unpar, ElTokenType type, ElStringView lexeme) {
    ElStringView cloned = el_dynarena_clone_sv(unpar->arena, lexeme);
    if (el_sv_is_null(cloned) && !el_sv_is_null(lexeme) && lexeme.len > 0) return false;

    ElToken tok = {
        .type = type,
        .lexeme = cloned,
        .span = EL_SRCSPAN_NULL,
    };
    return el_tkbuf_push(unpar->out, tok);
}

// if you're seeing this in 2027 or later,
//  increment the counter
// if you're seeing this in 2038,
//  get ready for the end of the world
#define BUFSIZE 2026

bool el_unparser_push_fmt(ElUnparser* unpar, ElTokenType type, const char* fmt, ...) {
    // NUCLEAR OPTIMIZATION BEGINS HERE
    char buf[BUFSIZE];

    va_list args;
    va_start(args, fmt);
    int n = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    if (n < 0) return false;
    if ((usize)n < sizeof(buf)) {
        // unparser_push clones the sv so this is safe.
        return el_unparser_push(unpar, type, el_sv_from_data_and_len(buf, (usize)n));
    }
    // NUCLEAR OPTIMIZATION ENDS HERE

    char* heap = el_dynarena_alloc(unpar->arena, (usize)n + 1, 1);
    if (heap == NULL) return false;

    // TIL you can call va_start multiple times
    // (this is THEORETICALLY faster than copying
    //  the va_list cuz most of the time this code
    //  path is unreachable and we don't need to do
    //  any additional work in the "fast" path)
    va_start(args, fmt);
    vsnprintf(heap, (usize)n + 1, fmt, args);
    va_end(args);

    return el_tkbuf_push(unpar->out, (ElToken) {
        .type = type,
        .lexeme = el_sv_from_data_and_len(heap, (usize)n),
    });
}

bool el_unparser_push_punct(ElUnparser* unpar, ElTokenType type) {
    ElStringView lexeme;
    switch (type) {
    case EL_TT_PLUS:               lexeme = EL_SV("+");     break;
    case EL_TT_MINUS:              lexeme = EL_SV("-");     break;
    case EL_TT_STAR:               lexeme = EL_SV("*");     break;
    case EL_TT_SLASH:              lexeme = EL_SV("/");     break;
    case EL_TT_PERCENT:            lexeme = EL_SV("%");     break;
    case EL_TT_CARET:              lexeme = EL_SV("^");     break;
    case EL_TT_INC:                lexeme = EL_SV("++");    break;
    case EL_TT_DEC:                lexeme = EL_SV("--");    break;
    case EL_TT_ASSIGN:             lexeme = EL_SV("=");     break;
    case EL_TT_ADD_ASSIGN:         lexeme = EL_SV("+=");    break;
    case EL_TT_SUB_ASSIGN:         lexeme = EL_SV("-=");    break;
    case EL_TT_MUL_ASSIGN:         lexeme = EL_SV("*=");    break;
    case EL_TT_DIV_ASSIGN:         lexeme = EL_SV("/=");    break;
    case EL_TT_MOD_ASSIGN:         lexeme = EL_SV("%=");    break;
    case EL_TT_EQL:                lexeme = EL_SV("==");    break;
    case EL_TT_NEQ:                lexeme = EL_SV("!=");    break;
    case EL_TT_LT:                 lexeme = EL_SV("<");     break;
    case EL_TT_LTE:                lexeme = EL_SV("<=");    break;
    case EL_TT_GT:                 lexeme = EL_SV(">");     break;
    case EL_TT_GTE:                lexeme = EL_SV(">=");    break;
    case EL_TT_LOGICAL_AND:        lexeme = EL_SV("&&");    break;
    case EL_TT_LOGICAL_OR:         lexeme = EL_SV("||");    break;
    case EL_TT_LOGICAL_NOT:        lexeme = EL_SV("!");     break;
    case EL_TT_LOGICAL_IMP:        lexeme = EL_SV("=>");    break;
    case EL_TT_LOGICAL_AND_ASSIGN: lexeme = EL_SV("&&=");   break;
    case EL_TT_LOGICAL_OR_ASSIGN:  lexeme = EL_SV("||=");   break;
    case EL_TT_LOGICAL_IMP_ASSIGN: lexeme = EL_SV("=>=");   break;
    case EL_TT_BITWISE_AND:        lexeme = EL_SV("&");     break;
    case EL_TT_BITWISE_OR:         lexeme = EL_SV("|");     break;
    case EL_TT_BITWISE_XOR:        lexeme = EL_SV("<>");    break;
    case EL_TT_BITWISE_NOT:        lexeme = EL_SV("~");     break;
    case EL_TT_BITWISE_IMP:        lexeme = EL_SV("~>");    break;
    case EL_TT_BITWISE_AND_ASSIGN: lexeme = EL_SV("&=");    break;
    case EL_TT_BITWISE_OR_ASSIGN:  lexeme = EL_SV("|=");    break;
    case EL_TT_BITWISE_XOR_ASSIGN: lexeme = EL_SV("<>=");   break;
    case EL_TT_BITWISE_IMP_ASSIGN: lexeme = EL_SV("~>=");   break;
    case EL_TT_SHL:                lexeme = EL_SV("<<");    break;
    case EL_TT_SHR:                lexeme = EL_SV(">>");    break;
    case EL_TT_SHL_ASSIGN:         lexeme = EL_SV("<<=");   break;
    case EL_TT_SHR_ASSIGN:         lexeme = EL_SV(">>=");   break;
    case EL_TT_LPAREN:             lexeme = EL_SV("(");     break;
    case EL_TT_RPAREN:             lexeme = EL_SV(")");     break;
    case EL_TT_LBRACKET:           lexeme = EL_SV("[");     break;
    case EL_TT_RBRACKET:           lexeme = EL_SV("]");     break;
    case EL_TT_LBRACE:             lexeme = EL_SV("{");     break;
    case EL_TT_RBRACE:             lexeme = EL_SV("}");     break;
    case EL_TT_SEMICOLON:          lexeme = EL_SV(";");     break;
    case EL_TT_COLON:              lexeme = EL_SV(":");     break;
    case EL_TT_DOUBLECOLON:        lexeme = EL_SV("::");    break;
    case EL_TT_COMMA:              lexeme = EL_SV(",");     break;
    case EL_TT_DOT:                lexeme = EL_SV(".");     break;
    case EL_TT_OPT:                lexeme = EL_SV("?");     break;
    case EL_TT_OPT_FB:             lexeme = EL_SV("\?\?");  break;
    case EL_TT_OPT_FB_ASSIGN:      lexeme = EL_SV("\?\?="); break;
    case EL_TT_OPT_MAP:            lexeme = EL_SV("?>");    break;
    case EL_TT_OPT_DOT:            lexeme = EL_SV("?.");    break;
    case EL_TT_HASH:               lexeme = EL_SV("#");     break;
    case EL_TT_ELLIPSIS:           lexeme = EL_SV("...");   break;
    case EL_TT_EOF:                lexeme = EL_SV_NULL;     break;
    default:                       return false;
    }
    return el_unparser_push(unpar, type, lexeme);
}

bool el_unparser_push_kw(ElUnparser* unpar, ElTokenType type) {
    ElStringView lexeme;
    switch (type) {
    case EL_TT_KW_IF:         lexeme = EL_SV("if");       break;
    case EL_TT_KW_ELSE:       lexeme = EL_SV("else");     break;
    case EL_TT_KW_SWITCH:     lexeme = EL_SV("switch");   break;
    case EL_TT_KW_WHILE:      lexeme = EL_SV("while");    break;
    case EL_TT_KW_FOR:        lexeme = EL_SV("for");      break;
    case EL_TT_KW_DO:         lexeme = EL_SV("do");       break;
    case EL_TT_KW_CONTINUE:   lexeme = EL_SV("continue"); break;
    case EL_TT_KW_DEFAULT:    lexeme = EL_SV("default");  break;
    case EL_TT_KW_RETURN:     lexeme = EL_SV("return");   break;
    case EL_TT_KW_BREAK:      lexeme = EL_SV("break");    break;
    case EL_TT_KW_CASE:       lexeme = EL_SV("case");     break;
    case EL_TT_KW_GOTO:       lexeme = EL_SV("goto");     break;
    case EL_TT_KW_EXTERN:     lexeme = EL_SV("extern");   break;
    case EL_TT_KW_INTERNAL:   lexeme = EL_SV("internal"); break;
    case EL_TT_KW_STATIC:     lexeme = EL_SV("static");   break;
    case EL_TT_KW_VOLATILE:   lexeme = EL_SV("volatile"); break;
    case EL_TT_KW_CONST:      lexeme = EL_SV("const");    break;
    case EL_TT_KW_WONLY:      lexeme = EL_SV("wonly");    break;
    case EL_TT_KW_AS:         lexeme = EL_SV("as");       break;
    case EL_TT_KW_BITCAST:    lexeme = EL_SV("bitcast");  break;
    case EL_TT_KW_ENUM:       lexeme = EL_SV("enum");     break;
    case EL_TT_KW_UNION:      lexeme = EL_SV("union");    break;
    case EL_TT_KW_STRUCT:     lexeme = EL_SV("struct");   break;
    case EL_TT_KW_TYPEDEF:    lexeme = EL_SV("typedef");  break;
    case EL_TT_KW_ALIAS:      lexeme = EL_SV("alias");    break;
    case EL_TT_TRUE_LITERAL:  lexeme = EL_SV("true");     break;
    case EL_TT_FALSE_LITERAL: lexeme = EL_SV("false");    break;
    case EL_TT_NULL_LITERAL:  lexeme = EL_SV("null");     break;
    default:                  return false;
    }
    return el_unparser_push(unpar, type, lexeme);
}

bool el_unparser_push_ident(ElUnparser* unpar, ElStringView name) {
    return el_unparser_push(unpar, EL_TT_IDENT, name);
}
