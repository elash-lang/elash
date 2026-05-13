#include <elash/ast/tree/expr/call.h>
#include <elash/ast/tree/expr.h>

#include <elash/util/assert.h>

ElAstExprNode* el_ast_new_call_expr(ElDynArena* arena, ElSourceSpan span, ElAstExprNode* callee, ElAstExprNode* args, usize arg_count) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstExprNode, {
        .type = EL_AST_EXPR_CALL,
        .span = span,
        .next = NULL,
        .as.call = {
            .callee = callee,
            .args = args,
            .arg_count = arg_count,
        },
    });
}
