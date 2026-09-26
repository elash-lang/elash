#pragma once

#include <elash/defs/int-types.h>
#include <elash/lexer/tokbuf.h>
#include <elash/util/dynarena.h>

#include <elash/ast/tree/expr.h>
#include <elash/ast/tree/stmt.h>
#include <elash/ast/tree/init.h>
#include <elash/ast/tree/decl.h>
#include <elash/ast/tree/module.h>
#include <elash/ast/tree/type.h>
#include <elash/ast/tree/toe.h>
#include <elash/ast/tree/toi.h>
#include <elash/ast/tree/unr.h>

typedef struct ElUnparser {
    ElTokenBuf* out;
    ElDynArena* arena;
} ElUnparser;

void el_unparser_init(ElUnparser* unparser, ElTokenBuf* out, ElDynArena* arena);

void el_unparser_push(ElUnparser* unparser, ElTokenType type, ElStringView lexeme);
void el_unparser_push_punct(ElUnparser* unparser, ElTokenType type);
void el_unparser_push_kw(ElUnparser* unparser, ElTokenType type);
void el_unparser_push_ident(ElUnparser* unparser, ElStringView name);
void el_unparser_push_fmt(ElUnparser* unparser, ElTokenType type, const char* fmt, ...);

void el_unparser_unparse_module(ElUnparser* unparser, ElAstModule* module);
void el_unparser_unparse_decl(ElUnparser* unparser, ElAstDecl* decl);
void el_unparser_unparse_stmt(ElUnparser* unparser, ElAstStmt* stmt);
void el_unparser_unparse_expr(ElUnparser* unparser, ElAstExpr* expr);
void el_unparser_unparse_type(ElUnparser* unparser, ElAstType* type);
void el_unparser_unparse_init(ElUnparser* unparser, ElAstInit* init);
void el_unparser_unparse_unr(ElUnparser* unparser,  ElAstUnr* unr);
void el_unparser_unparse_toe(ElUnparser* unparser,  ElAstToE* toe);
void el_unparser_unparse_toi(ElUnparser* unparser,  ElAstToI* toi);

void _el_unparser_unparse_ident(ElUnparser* unparser, ElAstIdent* ident);
void _el_unparser_unparse_block(ElUnparser* unparser, ElAstBlockStmt* block);
void _el_unparser_unparse_expr_prec(ElUnparser* unparser, ElAstExpr* expr, int min_prec);
void _el_unparser_unparse_type_base(ElUnparser* unparser, ElAstType* type);
void _el_unparser_unparse_func_sig(ElUnparser* unparser, ElAstFuncSignature* sig);
