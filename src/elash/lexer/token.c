#include <elash/lexer/token.h>

#include <elash/util/strbuf.h>
#include <stdlib.h>
#include <string.h>

#define F(T, M) \
    { EL_SV(T), EL_SV(M) }

static struct {
    ElStringView to_str;
    ElStringView format;
} el_token_type_to_string_map[] = {
    [EL_TT_EOF] = F("EOF", "EOF"),

    [EL_TT_IDENT] = F("IDENT", "identifier"),

    [EL_TT_INT_LITERAL]    = F("INT_LITERAL", "<int literal>"),
    [EL_TT_FLOAT_LITERAL]  = F("FLOAT_LITERAL", "<float literal>"),
    [EL_TT_CHAR_LITERAL]   = F("CHAR_LITERAL", "<char literal>"),
    [EL_TT_STRING_LITERAL] = F("STRING_LITERAL", "<string literal>"),

    [EL_TT_TRUE_LITERAL]  = F("TRUE_LITERAL", "true"),
    [EL_TT_FALSE_LITERAL] = F("FALSE_LITERAL", "false"),
    [EL_TT_NULL_LITERAL]  = F("NULL_LITERAL", "null"),

    [EL_TT_KW_IF]     = F("KW_IF", "if"),
    [EL_TT_KW_ELSE]   = F("KW_ELSE", "else"),
    [EL_TT_KW_SWITCH] = F("KW_SWITCH", "switch"),

    [EL_TT_KW_WHILE] = F("KW_WHILE", "while"),
    [EL_TT_KW_FOR]   = F("KW_FOR", "for"),
    [EL_TT_KW_DO]    = F("KW_DO", "do"),

    [EL_TT_KW_CONTINUE] = F("KW_CONTINUE", "continue"),
    [EL_TT_KW_DEFAULT]  = F("KW_DEFAULT", "default"),
    [EL_TT_KW_RETURN]   = F("KW_RETURN", "return"),
    [EL_TT_KW_BREAK]    = F("KW_BREAK", "break"),
    [EL_TT_KW_CASE]     = F("KW_CASE", "case"),
    [EL_TT_KW_GOTO]     = F("KW_GOTO", "goto"),

    [EL_TT_KW_EXTERN] = F("KW_EXTERN", "extern"),
    [EL_TT_KW_STATIC] = F("KW_STATIC", "static"),
    [EL_TT_KW_INLINE] = F("KW_INLINE", "inline"),

    [EL_TT_KW_AS] = F("KW_AS", "as"),

    [EL_TT_KW_VOLATILE] = F("KW_VOLATILE", "volatile"),
    [EL_TT_KW_CONST] = F("KW_CONST", "const"),
    [EL_TT_KW_WRITE] = F("KW_WRITE", "write"),

    [EL_TT_KW_ENUM]    = F("KW_ENUM", "enum"),
    [EL_TT_KW_UNION]   = F("KW_UNION", "union"),
    [EL_TT_KW_STRUCT]  = F("KW_STRUCT", "struct"),
    [EL_TT_KW_TYPEDEF] = F("KW_TYPEDEF", "typedef"),

    [EL_TT_SIZEOF] = F("SIZEOF", "sizeof"),

    [EL_TT_PLUS]    = F("PLUS", "'+'"),
    [EL_TT_MINUS]   = F("MINUS", "'-'"),
    [EL_TT_STAR]    = F("STAR", "'*'"),
    [EL_TT_SLASH]   = F("SLASH", "'/'"),
    [EL_TT_PERCENT] = F("PERCENT", "'/'"),
    [EL_TT_CARET]   = F("CARET", "'^'"),

    [EL_TT_INC] = F("INC", "'++'"),
    [EL_TT_DEC] = F("DEC", "'--'"),

    [EL_TT_ASSIGN] =     F("ASSIGN",     "'='"),
    [EL_TT_ADD_ASSIGN] = F("ADD_ASSIGN", "'+='"),
    [EL_TT_SUB_ASSIGN] = F("SUB_ASSIGN", "'-='"),
    [EL_TT_MUL_ASSIGN] = F("MUL_ASSIGN", "'*='"),
    [EL_TT_DIV_ASSIGN] = F("DIV_ASSIGN", "'/='"),
    [EL_TT_MOD_ASSIGN] = F("MOD_ASSIGN", "'%='"),

    [EL_TT_EQL] = F("EQL", "'=='"),
    [EL_TT_NEQ] = F("NEQ", "'!='"),
    [EL_TT_LT]  = F("LT", "'<'"),
    [EL_TT_LTE] = F("LTE", "'<='"),
    [EL_TT_GT]  = F("GT", "'>'"),
    [EL_TT_GTE] = F("GTE", "'>='"),

    [EL_TT_LOGICAL_AND] = F("LOGICAL_AND", "'&&'"),
    [EL_TT_LOGICAL_OR]  = F("LOGICAL_OR",  "'||'"),
    [EL_TT_LOGICAL_NOT] = F("LOGICAL_NOT", "'!'"),
    [EL_TT_LOGICAL_IMP] = F("LOGICAL_IMP", "'=>'"),

    [EL_TT_LOGICAL_AND_ASSIGN] = F("LOGICAL_AND_ASSIGN", "'&&='"),
    [EL_TT_LOGICAL_OR_ASSIGN]  = F("LOGICAL_OR_ASSIGN",  "'||='"),
    [EL_TT_LOGICAL_IMP_ASSIGN] = F("LOGICAL_IMP_ASSIGN", "'=>='"),

    [EL_TT_BITWISE_AND] = F("BITWISE_AND", "'&'"),
    [EL_TT_BITWISE_OR]  = F("BITWISE_OR", "'|'"),
    [EL_TT_BITWISE_NOT] = F("BITWISE_NOT", "'~'"),
    [EL_TT_BITWISE_XOR] = F("BITWISE_XOR", "'<>'"),
    [EL_TT_BITWISE_IMP] = F("BITWISE_IMP", "'~>'"),

    [EL_TT_BITWISE_AND_ASSIGN] = F("BITWISE_AND_ASSIGN", "'&='"),
    [EL_TT_BITWISE_OR_ASSIGN]  = F("BITWISE_OR_ASSIGN",  "'|='"),
    [EL_TT_BITWISE_XOR_ASSIGN] = F("BITWISE_XOR_ASSIGN", "'<>='"),
    [EL_TT_BITWISE_IMP_ASSIGN] = F("BITWISE_IMP_ASSIGN", "'~>='"),

    [EL_TT_SHL] = F("SHL", "'<<'"),
    [EL_TT_SHR] = F("SHR", "'>>'"),

    [EL_TT_SHL_ASSIGN] = F("SHL_ASSIGN", "'<<='"),
    [EL_TT_SHR_ASSIGN] = F("SHR_ASSIGN", "'>>='"),

    [EL_TT_LPAREN]   = F("LPAREN", "'('"),
    [EL_TT_RPAREN]   = F("RPAREN", "')'"),
    [EL_TT_LBRACKET] = F("LBRACKET", "'['"),
    [EL_TT_RBRACKET] = F("RBRACKET", "']'"),
    [EL_TT_LBRACE]   = F("LBRACE", "'{'"),
    [EL_TT_RBRACE]   = F("RBRACE", "'}'"),

    [EL_TT_SEMICOLON]   = F("SEMICOLON", "';'"),
    [EL_TT_COLON]       = F("COLON", "':'"),
    [EL_TT_COMMA]       = F("COMMA", "','"),
    [EL_TT_DOT]         = F("DOT", "'.'"),
    [EL_TT_DOUBLECOLON] = F("DOUBLECOLON", "'::'"),

    [EL_TT_HASH] = F("HASH", "'#'"),

    [EL_TT_PP_INCLUDE] = F("PP_INCLUDE", "include directive"),
    [EL_TT_PP_EMBED]   = F("PP_EMBED", "embed directive"),

    [EL_TT_PP_EMIT] = F("PP_EMIT", "emit directive"),
    [EL_TT_PP_END]  = F("PP_END", "end directive"),

    [EL_TT_PP_DEFINE] = F("PP_DEFINE", "define directive"),
    [EL_TT_PP_UNDEF]  = F("PP_UNDEF", "undef directive"),

    [EL_TT_PP_PRAGMA]  = F("PP_PRAGMA", "pragma directive"),
    [EL_TT_PP_ERROR]   = F("PP_ERROR", "error directive"),
    [EL_TT_PP_WARNING] = F("PP_WARNING", "warning directive"),
    [EL_TT_PP_NOTE]    = F("PP_NOTE", "note directive"),
    [EL_TT_PP_DEBUG]   = F("PP_DEBUG", "debug directive"),

    [EL_TT_PP_ASSIGN] = F("PP_ASSIGN", "assign directive"),
    [EL_TT_PP_INC]    = F("PP_INC", "inc directive"),
    [EL_TT_PP_DEC]    = F("PP_DEC", "dec directive"),

    [EL_TT_PP_IF]   = F("PP_IF", "if directive"),
    [EL_TT_PP_ELIF] = F("PP_ELIF", "elif directive"),
    [EL_TT_PP_ELSE] = F("PP_ELSE", "else directive"),

    [EL_TT_PP_WHILE]   = F("PP_WHILE", "while directive"),
    [EL_TT_PP_FOR]     = F("PP_FOR", "for directive"),
    [EL_TT_PP_FOREACH] = F("PP_FOREACH", "foreach directive"),

    [EL_TT_ELLIPSIS]      = F("ELLIPSIS", "'...'"),
    [EL_TT_LINE_COMMENT]  = F("LINE_COMMENT", "line comment"),
    [EL_TT_BLOCK_COMMENT] = F("BLOCK_COMMENT", "block comment"),

    [EL_TT_WHITESPACE] = F("WHITESPACE", "whitespace"),
    [EL_TT_NEWLINE]    = F("NEWLINE", "new line"),

    [EL_TT_UNKNOWN] = F("UNKNOWN", "unknown"),
};

bool el_token_type_is_keyword(ElTokenType tt) {
    return tt > _EL_TT_GROUP_BEGIN_KEYWORDS && tt < _EL_TT_GROUP_END_KEYWORDS;
}

ElStringView el_token_type_to_string(ElTokenType tt) {
    if (tt < 0 || tt >= _EL_TT_COUNT) return EL_SV_NULL;
    ElStringView s = el_token_type_to_string_map[tt].to_str;
    if (el_sv_is_null(s)) return EL_SV("UNKNOWN");
    return s;
}

ElStringView el_token_type_format(ElTokenType tt) {
    if (tt < 0 || tt >= _EL_TT_COUNT) return EL_SV_NULL;
    ElStringView s = el_token_type_to_string_map[tt].format;
    if (el_sv_is_null(s)) return EL_SV("unknown");
    return s;
}

usize el_token_debug_format(const ElToken* tok, usize n, char buf[static n]) {
    ElStringView type_string = el_token_type_to_string(tok->type);
    usize count = 0;

    usize copy_len = (type_string.len < n - 1) ? type_string.len : n - 1;
    memcpy(buf, type_string.data, copy_len);
    count += copy_len;

    if (!el_sv_is_null(tok->lexeme) && count < n - 2) {
        buf[count++] = '(';
        usize lex_len = (tok->lexeme.len < n - count - 1) ? tok->lexeme.len : n - count - 1;
        memcpy(buf + count, tok->lexeme.data, lex_len);
        count += lex_len;
        if (count < n - 1) {
            buf[count++] = ')';
        }
    }

    buf[count < n ? count : n - 1] = '\0';
    return count;
}

usize el_token_to_debug_string(const ElToken* tok, char** out) {
    ElStringView type_string = el_token_type_to_string(tok->type);

    if (el_sv_is_null(tok->lexeme)) {
        *out = malloc(type_string.len + 1); // +1 for \0
        *out = memcpy(*out, type_string.data, type_string.len + 1);
        return type_string.len;
    }

    const usize full_len = type_string.len + 1 // '('
                         + tok->lexeme.len + 1; // ')'

    *out = malloc(full_len + 1); // +1 for \0
    memcpy(*out, type_string.data, type_string.len);
    (*out)[type_string.len] = '(';
    memcpy(*out + type_string.len + 1, tok->lexeme.data, tok->lexeme.len);
    (*out)[type_string.len + 1 + tok->lexeme.len] = ')';
    (*out)[full_len] = '\0';
    return type_string.len;
}

usize el_token_print(const ElToken* tok, FILE* out) {
    usize bytes_written = 0;
    ElStringView type_string = el_token_type_to_string(tok->type);

    bytes_written += el_sv_print(type_string, out);

    if (!el_sv_is_null(tok->lexeme)) {
        bytes_written += fputc('(', out) == EOF ? 0 : 1;
        bytes_written += el_sv_print(tok->lexeme, out);
        bytes_written += fputc(')', out) == EOF ? 0 : 1;
    }

    return bytes_written;
}

bool el_token_to_raw_string(const ElToken* tok, ElStringBuf* sb) {
    bool success = true;

    switch (tok->type) {
    case EL_TT_STRING_LITERAL:
        success &= el_strbuf_append_char(sb, '"');
        success &= el_strbuf_append(sb, tok->lexeme);
        success &= el_strbuf_append_char(sb, '"');
        break;
    case EL_TT_CHAR_LITERAL:
        success &= el_strbuf_append_char(sb, '\'');
        success &= el_strbuf_append(sb, tok->lexeme);
        success &= el_strbuf_append_char(sb, '\'');
        break;
    case EL_TT_LINE_COMMENT:
        success &= el_strbuf_append(sb, EL_SV("//"));
        success &= el_strbuf_append(sb, tok->lexeme);
        break;
    case EL_TT_BLOCK_COMMENT:
        success &= el_strbuf_append(sb, EL_SV("/*"));
        success &= el_strbuf_append(sb, tok->lexeme);
        success &= el_strbuf_append(sb, EL_SV("*/"));
        break;
    case EL_TT_NEWLINE:
        success &= el_strbuf_append_char(sb, '\n');
        break;

    case EL_TT_UNKNOWN:
    case EL_TT_EOF:
        break;
    default:
        // for all other token types, append the lexeme directly
        success &= el_strbuf_append(sb, tok->lexeme);
        break;
    }

    if (el_token_type_is_keyword(tok->type) || tok->type == EL_TT_IDENT) {
        success &= el_strbuf_append_char(sb, ' ');
    }

    return success;
}

