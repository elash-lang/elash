#pragma once

#include <elash/util/dynarena.h>
#include <elash/defs/sv.h>
#include <elash/srcdoc/span.h>

typedef struct ElAstExprNode ElAstExprNode;

typedef enum ElAstLiteralType {
    EL_AST_LIT_INT,
    EL_AST_LIT_FLOAT,
    EL_AST_LIT_CHAR,
    EL_AST_LIT_STRING,
    EL_AST_LIT_BOOL,
    EL_AST_LIT_NULL,
} ElAstLiteralType;

typedef struct ElAstIntLiteralNode {
    int64_t value;
} ElAstIntLiteralNode;

typedef struct ElAstFloatLiteralNode {
    long double value;
} ElAstFloatLiteralNode;

typedef struct ElAstCharLiteralNode {
    char value;
} ElAstCharLiteralNode;

typedef struct ElAstStringLiteralNode {
    ElStringView value;
} ElAstStringLiteralNode;

typedef struct ElAstBoolLiteralNode {
    bool value;
} ElAstBoolLiteralNode;

typedef struct ElAstLiteralNode {
    ElAstLiteralType type;
    union {
        ElAstIntLiteralNode int_;
        ElAstFloatLiteralNode float_;
        ElAstCharLiteralNode char_;
        ElAstStringLiteralNode str_;
        ElAstBoolLiteralNode bool_;
    } of;
} ElAstLiteralNode;

ElAstLiteralNode el_ast_int_literal(int64_t value);
ElAstLiteralNode el_ast_float_literal(long double value);
ElAstLiteralNode el_ast_char_literal(char value);
ElAstLiteralNode el_ast_string_literal(ElStringView value);
ElAstLiteralNode el_ast_bool_literal(bool value);
ElAstLiteralNode el_ast_null_literal();

ElAstExprNode* el_ast_new_int_literal(ElDynArena* arena, ElSourceSpan span, int64_t value);
ElAstExprNode* el_ast_new_float_literal(ElDynArena* arena, ElSourceSpan span, long double value);
ElAstExprNode* el_ast_new_char_literal(ElDynArena* arena, ElSourceSpan span, char value);
ElAstExprNode* el_ast_new_string_literal(ElDynArena* arena, ElSourceSpan span, ElStringView value);
ElAstExprNode* el_ast_new_bool_literal(ElDynArena* arena, ElSourceSpan span, bool value);
ElAstExprNode* el_ast_new_null_literal(ElDynArena* arena, ElSourceSpan span);
