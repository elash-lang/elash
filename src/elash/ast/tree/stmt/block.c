#include <elash/ast/tree/stmt.h>

ElAstBlockStmt el_ast_block_stmt(ElAstStmt* stmts) {
    return (ElAstBlockStmt) {
        .stmts = stmts,
    };
}

ElAstStmt* el_ast_new_block_stmt(ElDynArena* arena, ElSourceSpan span, ElAstStmt* stmts) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstStmt, {
        .type = EL_AST_STMT_BLOCK,
        .span = span,
        .next = NULL,
        .as.block = el_ast_block_stmt(stmts),
    });
}
