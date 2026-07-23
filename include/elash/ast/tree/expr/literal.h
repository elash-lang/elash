#pragma once

#include <elash/util/dynarena.h>
#include <elash/defs/sv.h>
#include <elash/srcdoc/span.h>

typedef struct ElAstExpr ElAstExpr;

typedef enum ElAstLiteralType {
    EL_AST_LIT_INT,
    EL_AST_LIT_FLOAT,
    EL_AST_LIT_CHAR,
    EL_AST_LIT_STRING,
    EL_AST_LIT_BOOL,
    EL_AST_LIT_NULL,
} ElAstLiteralType;

typedef struct ElAstIntLiteral {
    // TODO: use arbitrary precision integers
    int64_t value;
} ElAstIntLiteral;

typedef struct ElAstFloatLiteral {
    // TODO: use arbitrary precision floats
    double value;
} ElAstFloatLiteral;

typedef struct ElAstCharLiteral {
    char value;
} ElAstCharLiteral;

typedef struct ElAstStringLiteral {
    ElStringView value;
} ElAstStringLiteral;

typedef struct ElAstBoolLiteral {
    bool value;
} ElAstBoolLiteral;

typedef struct ElAstLiteral {
    ElAstLiteralType type;
    union {
        ElAstIntLiteral int_;
        ElAstFloatLiteral float_;
        ElAstCharLiteral char_;
        ElAstStringLiteral str_;
        ElAstBoolLiteral bool_;
    } of;
} ElAstLiteral;

ElAstLiteral el_ast_int_literal(int64_t value);
ElAstLiteral el_ast_float_literal(long double value);
ElAstLiteral el_ast_char_literal(char value);
ElAstLiteral el_ast_string_literal(ElStringView value);
ElAstLiteral el_ast_bool_literal(bool value);
ElAstLiteral el_ast_null_literal();

ElAstExpr* el_ast_new_int_literal(ElDynArena* arena, ElSourceSpan span, int64_t value);
ElAstExpr* el_ast_new_float_literal(ElDynArena* arena, ElSourceSpan span, long double value);
ElAstExpr* el_ast_new_char_literal(ElDynArena* arena, ElSourceSpan span, char value);
ElAstExpr* el_ast_new_string_literal(ElDynArena* arena, ElSourceSpan span, ElStringView value);
ElAstExpr* el_ast_new_bool_literal(ElDynArena* arena, ElSourceSpan span, bool value);
ElAstExpr* el_ast_new_null_literal(ElDynArena* arena, ElSourceSpan span);
