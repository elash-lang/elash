#pragma once

#include <elash/util/dynarena.h>
#include <elash/srcdoc/span.h>

typedef struct ElAstStmt ElAstStmt;

typedef struct ElAstBlockStmt {
    ElAstStmt* stmts;
} ElAstBlockStmt;

ElAstBlockStmt el_ast_block_stmt(ElAstStmt* stmts);
ElAstStmt* el_ast_new_block_stmt(ElDynArena* arena, ElSourceSpan span, ElAstStmt* stmts);
