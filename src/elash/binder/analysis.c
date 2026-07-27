#include <elash/binder/binder.h>

bool _el_binder_stmt_always_returns(ElBinder* binder, ElHirStmt* stmt) {
    (void) binder;
    switch (stmt->kind) {
    case EL_HIR_STMT_RETURN:
        return true;
    case EL_HIR_STMT_BLOCK:
        return _el_binder_block_always_returns(binder, stmt->as.block);

    case EL_HIR_STMT_IF:
        if (stmt->as.if_.else_ == NULL) return false;
        return _el_binder_stmt_always_returns(binder, stmt->as.if_.then)
            && _el_binder_stmt_always_returns(binder, stmt->as.if_.else_);
    case EL_HIR_STMT_WHILE:
        return _el_binder_stmt_always_returns(binder, stmt->as.while_.body);

    default:
        return false;
    }
}

bool _el_binder_block_always_returns(ElBinder* binder, ElHirBlockStmt block) {
    for (ElHirStmt* stmt = block.stmts; stmt != NULL; stmt = stmt->next) {
        if (_el_binder_stmt_always_returns(binder, stmt)) {
            return true;
        }
    }
    return false;
}
