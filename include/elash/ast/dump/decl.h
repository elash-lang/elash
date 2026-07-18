#pragma once

#include <elash/ast/tree/decl.h>
#include <elash/defs/int-types.h>
#include <stdio.h>

void el_ast_dump_decl(ElAstDecl* node, usize indent, FILE* out);
