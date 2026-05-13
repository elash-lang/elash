#include <elash/ast/tree/expr/call.h>
#include <elash/ast/tree/expr.h>

#include <elash/util/assert.h>

ElAstCallExprNode el_ast_call_expr(ElAstExprNode* callee, ElAstExprNode* args, usize arg_count) {
    return (ElAstCallExprNode) {
        .callee = callee,
        .args = args,
        .arg_count = arg_count,
    };
}

ElAstExprNode* el_ast_new_call_expr(ElDynArena* arena, ElSourceSpan span, ElAstExprNode* callee, ElAstExprNode* args, usize arg_count) {
    ElAstExprNode* node = EL_DYNARENA_NEW(arena, ElAstExprNode);
    node->type = EL_AST_EXPR_CALL;
    node->span = span;
    node->as.call = el_ast_call_expr(callee, args, arg_count);
    node->next = NULL;
    return node;
}
