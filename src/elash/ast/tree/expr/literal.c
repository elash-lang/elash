#include <elash/ast/tree/expr/literal.h>
#include <elash/ast/tree/expr.h>

static ElAstExpr* el_ast_new_literal_wrapper(ElDynArena* arena, ElSourceSpan span, ElAstLiteral lit) {
    ElAstExpr* node = EL_DYNARENA_NEW(arena, ElAstExpr);
    node->type = EL_AST_EXPR_LITERAL;
    node->span = span;
    node->as.literal = lit;
    node->next = NULL;
    return node;
}

ElAstLiteral el_ast_int_literal(int64_t value) {
    return (ElAstLiteral) {
        .type = EL_AST_LIT_INT,
        .of.int_.value = value,
    };
}

ElAstLiteral el_ast_float_literal(long double value) {
    return (ElAstLiteral) {
        .type = EL_AST_LIT_FLOAT,
        .of.float_.value = value,
    };
}

ElAstLiteral el_ast_char_literal(char value) {
    return (ElAstLiteral) {
        .type = EL_AST_LIT_CHAR,
        .of.char_.value = value,
    };
}

ElAstLiteral el_ast_string_literal(ElStringView value) {
    return (ElAstLiteral) {
        .type = EL_AST_LIT_STRING,
        .of.str_.value = value,
    };
}

ElAstLiteral el_ast_bool_literal(bool value) {
    return (ElAstLiteral) {
        .type = EL_AST_LIT_BOOL,
        .of.bool_.value = value,
    };
}

ElAstLiteral el_ast_null_literal(void) {
    return (ElAstLiteral) {
        .type = EL_AST_LIT_NULL,
    };
}

ElAstExpr* el_ast_new_int_literal(ElDynArena* arena, ElSourceSpan span, int64_t value) {
    return el_ast_new_literal_wrapper(arena, span, el_ast_int_literal(value));
}

ElAstExpr* el_ast_new_float_literal(ElDynArena* arena, ElSourceSpan span, long double value) {
    return el_ast_new_literal_wrapper(arena, span, el_ast_float_literal(value));
}

ElAstExpr* el_ast_new_char_literal(ElDynArena* arena, ElSourceSpan span, char value) {
    return el_ast_new_literal_wrapper(arena, span, el_ast_char_literal(value));
}

ElAstExpr* el_ast_new_string_literal(ElDynArena* arena, ElSourceSpan span, ElStringView value) {
    return el_ast_new_literal_wrapper(arena, span, el_ast_string_literal(value));
}

ElAstExpr* el_ast_new_bool_literal(ElDynArena* arena, ElSourceSpan span, bool value) {
    return el_ast_new_literal_wrapper(arena, span, el_ast_bool_literal(value));
}

ElAstExpr* el_ast_new_null_literal(ElDynArena* arena, ElSourceSpan span) {
    return el_ast_new_literal_wrapper(arena, span, el_ast_null_literal());
}
