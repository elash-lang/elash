#include <elash/ast/tree/stmt.h>

ElAstBlockStmtNode el_ast_block_stmt(ElAstStmtNode* stmts) {
    return (ElAstBlockStmtNode) {
        .stmts = stmts,
    };
}

ElAstStmtNode* el_ast_new_block_stmt(ElDynArena* arena, ElSourceSpan span, ElAstStmtNode* stmts) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstStmtNode, {
        .type = EL_AST_STMT_BLOCK,
        .span = span,
        .next = NULL,
        .as.block = el_ast_block_stmt(stmts),
    });
}
