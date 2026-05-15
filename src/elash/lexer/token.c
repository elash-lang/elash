#include <elash/lexer/token.h>

#include <elash/util/strbuf.h>
#include <stdlib.h>
#include <string.h>

static ElStringView el_token_type_to_string_map[] = {
    [EL_TT_EOF] = EL_SV("EOF"),

    [EL_TT_IDENT] = EL_SV("IDENT"),

    [EL_TT_INT_LITERAL] = EL_SV("INT_LITERAL"),
    [EL_TT_FLOAT_LITERAL] = EL_SV("FLOAT_LITERAL"),
    [EL_TT_CHAR_LITERAL] = EL_SV("CHAR_LITERAL"),
    [EL_TT_STRING_LITERAL] = EL_SV("STRING_LITERAL"),

    [EL_TT_TRUE_LITERAL] = EL_SV("TRUE_LITERAL"),
    [EL_TT_FALSE_LITERAL] = EL_SV("FALSE_LITERAL"),
    [EL_TT_NULL_LITERAL] = EL_SV("NULL_LITERAL"),

    [EL_TT_KW_IF] = EL_SV("KW_IF"),
    [EL_TT_KW_ELSE] = EL_SV("KW_ELSE"),
    [EL_TT_KW_SWITCH] = EL_SV("KW_SWITCH"),

    [EL_TT_KW_WHILE] = EL_SV("KW_WHILE"),
    [EL_TT_KW_FOR] = EL_SV("KW_FOR"),
    [EL_TT_KW_DO] = EL_SV("KW_DO"),

    [EL_TT_KW_CONTINUE] = EL_SV("KW_CONTINUE"),
    [EL_TT_KW_DEFAULT] = EL_SV("KW_DEFAULT"),
    [EL_TT_KW_RETURN] = EL_SV("KW_RETURN"),
    [EL_TT_KW_BREAK] = EL_SV("KW_BREAK"),
    [EL_TT_KW_CASE] = EL_SV("KW_CASE"),
    [EL_TT_KW_GOTO] = EL_SV("KW_GOTO"),

    [EL_TT_KW_EXTERN] = EL_SV("KW_EXTERN"),
    [EL_TT_KW_STATIC] = EL_SV("KW_STATIC"),
    [EL_TT_KW_INLINE] = EL_SV("KW_INLINE"),

    [EL_TT_KW_VOLATILE] = EL_SV("KW_VOLATILE"),
    [EL_TT_KW_CONST] = EL_SV("KW_CONST"),

    [EL_TT_KW_ENUM] = EL_SV("KW_ENUM"),
    [EL_TT_KW_UNION] = EL_SV("KW_UNION"),
    [EL_TT_KW_STRUCT] = EL_SV("KW_STRUCT"),
    [EL_TT_KW_TYPEDEF] = EL_SV("KW_TYPEDEF"),

    [EL_TT_SIZEOF] = EL_SV("SIZEOF"),

    [EL_TT_PLUS] = EL_SV("PLUS"),
    [EL_TT_MINUS] = EL_SV("MINUS"),
    [EL_TT_STAR] = EL_SV("STAR"),
    [EL_TT_SLASH] = EL_SV("SLASH"),
    [EL_TT_PERCENT] = EL_SV("PERCENT"),

    [EL_TT_INC] = EL_SV("INC"),
    [EL_TT_DEC] = EL_SV("DEC"),

    [EL_TT_ASSIGN] = EL_SV("ASSIGN"),
    [EL_TT_ADD_ASSIGN] = EL_SV("ADD_ASSIGN"),
    [EL_TT_SUB_ASSIGN] = EL_SV("SUB_ASSIGN"),
    [EL_TT_MUL_ASSIGN] = EL_SV("MUL_ASSIGN"),
    [EL_TT_DIV_ASSIGN] = EL_SV("DIV_ASSIGN"),
    [EL_TT_MOD_ASSIGN] = EL_SV("MOD_ASSIGN"),

    [EL_TT_EQL] = EL_SV("EQL"),
    [EL_TT_NEQ] = EL_SV("NEQ"),
    [EL_TT_LT] = EL_SV("LT"),
    [EL_TT_LTE] = EL_SV("LTE"),
    [EL_TT_GT] = EL_SV("GT"),
    [EL_TT_GTE] = EL_SV("GTE"),

    [EL_TT_LOGICAL_AND] = EL_SV("LOGICAL_AND"),
    [EL_TT_LOGICAL_OR] = EL_SV("LOGICAL_OR"),
    [EL_TT_LOGICAL_NOT] = EL_SV("LOGICAL_NOT"),

    [EL_TT_BITWISE_AND] = EL_SV("BITWISE_AND"),
    [EL_TT_BITWISE_OR] = EL_SV("BITWISE_OR"),
    [EL_TT_BITWISE_XOR] = EL_SV("BITWISE_XOR"),
    [EL_TT_BITWISE_NOT] = EL_SV("BITWISE_NOT"),

    [EL_TT_BITWISE_AND_ASSIGN] = EL_SV("BITWISE_AND_ASSIGN"),
    [EL_TT_BITWISE_OR_ASSIGN] = EL_SV("BITWISE_OR_ASSIGN"),
    [EL_TT_BITWISE_XOR_ASSIGN] = EL_SV("BITWISE_XOR_ASSIGN"),

    [EL_TT_SHL] = EL_SV("SHL"),
    [EL_TT_SHR] = EL_SV("SHR"),

    [EL_TT_SHL_ASSIGN] = EL_SV("SHL_ASSIGN"),
    [EL_TT_SHR_ASSIGN] = EL_SV("SHR_ASSIGN"),

    [EL_TT_LPAREN] = EL_SV("LPAREN"),
    [EL_TT_RPAREN] = EL_SV("RPAREN"),
    [EL_TT_LBRACKET] = EL_SV("LBRACKET"),
    [EL_TT_RBRACKET] = EL_SV("RBRACKET"),
    [EL_TT_LBRACE] = EL_SV("LBRACE"),
    [EL_TT_RBRACE] = EL_SV("RBRACE"),

    [EL_TT_SEMICOLON] = EL_SV("SEMICOLON"),
    [EL_TT_COLON] = EL_SV("COLON"),
    [EL_TT_COMMA] = EL_SV("COMMA"),
    [EL_TT_DOT] = EL_SV("DOT"),
    [EL_TT_DOUBLECOLON] = EL_SV("DOUBLECOLON"),
    [EL_TT_ARROW] = EL_SV("ARROW"),

    [EL_TT_HASH] = EL_SV("HASH"),

    [EL_TT_PP_INCLUDE] = EL_SV("PP_INCLUDE"),
    [EL_TT_PP_EMBED] = EL_SV("PP_EMBED"),

    [EL_TT_PP_EMIT] = EL_SV("PP_EMIT"),
    [EL_TT_PP_END] = EL_SV("PP_END"),

    [EL_TT_PP_DEFINE] = EL_SV("PP_DEFINE"),
    [EL_TT_PP_UNDEF] = EL_SV("PP_UNDEF"),

    [EL_TT_PP_PRAGMA] = EL_SV("PP_PRAGMA"),
    [EL_TT_PP_ERROR] = EL_SV("PP_ERROR"),
    [EL_TT_PP_WARNING] = EL_SV("PP_WARNING"),
    [EL_TT_PP_NOTE] = EL_SV("PP_NOTE"),
    [EL_TT_PP_DEBUG] = EL_SV("PP_DEBUG"),

    [EL_TT_PP_ASSIGN] = EL_SV("PP_ASSIGN"),
    [EL_TT_PP_INC] = EL_SV("PP_INC"),
    [EL_TT_PP_DEC] = EL_SV("PP_DEC"),

    [EL_TT_PP_IF] = EL_SV("PP_IF"),
    [EL_TT_PP_ELIF] = EL_SV("PP_ELIF"),
    [EL_TT_PP_ELSE] = EL_SV("PP_ELSE"),

    [EL_TT_PP_WHILE] = EL_SV("PP_WHILE"),
    [EL_TT_PP_FOR] = EL_SV("PP_FOR"),
    [EL_TT_PP_FOREACH] = EL_SV("PP_FOREACH"),

    [EL_TT_ELLIPSIS] = EL_SV("ELLIPSIS"),
    [EL_TT_LINE_COMMENT] = EL_SV("LINE_COMMENT"),
    [EL_TT_BLOCK_COMMENT] = EL_SV("BLOCK_COMMENT"),

    [EL_TT_WHITESPACE] = EL_SV("WHITESPACE"),
    [EL_TT_NEWLINE] = EL_SV("NEWLINE"),

    [EL_TT_UNKNOWN] = EL_SV("UNKNOWN"),
};

bool el_token_type_is_keyword(ElTokenType tt) {
    return tt > _EL_TT_GROUP_BEGIN_KEYWORDS && tt < _EL_TT_GROUP_END_KEYWORDS;
}

ElStringView el_token_type_to_string(ElTokenType tt) {
    if (tt < 0 || tt >= _EL_TT_COUNT) return EL_SV_NULL;
    ElStringView s = el_token_type_to_string_map[tt];
    if (el_sv_is_null(s)) return EL_SV("UNKNOWN");
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

