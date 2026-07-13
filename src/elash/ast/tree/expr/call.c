#include <elash/ast/tree/expr/call.h>
#include <elash/ast/tree/expr.h>

#include <elash/util/assert.h>

ElAstExpr* el_ast_new_call_expr(ElDynArena* arena, ElSourceSpan span, ElAstExpr* callee, ElAstInit* args, usize arg_count) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstExpr, {
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
