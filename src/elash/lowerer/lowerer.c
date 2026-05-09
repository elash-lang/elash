#include <elash/lowerer/lowerer.h>

// TODO: implementation

void el_lowerer_init(ElLowerer* lw) {(void) lw;}
void el_lowerer_free(ElLowerer* lw) {(void) lw;}

ElMirValue*  el_lowerer_lower_expr(ElLowerer* lw, ElHirExprNode* hir) {(void) lw, (void) hir; return NULL; }
void         el_lowerer_lower_stmt(ElLowerer* lw, ElHirStmtNode* hir) {(void) lw, (void) hir;}
void         el_lowerer_lower_toplvl(ElLowerer* lw, ElHirTopLevelNode* hir) {(void) lw, (void) hir;}
ElMirModule* el_lowerer_lower_module(ElLowerer* lw, ElHirModule* hir) { (void) lw, (void) hir; return NULL; }
