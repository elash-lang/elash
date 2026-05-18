#pragma once

#include <elash/util/dynarena.h>
#include <elash/srcdoc/span.h>

typedef struct ElAstStmt ElAstStmt;
typedef struct ElAstContinueStmt {
} ElAstContinueStmt;

ElAstStmt* el_ast_new_continue_stmt(ElDynArena* arena, ElSourceSpan span);
