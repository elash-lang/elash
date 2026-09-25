#include <elash/util/dynarena.h>
#include <elash/ast/tree/module.h>
#include <elash/ast/tree/decl.h>
#include <elash/ast/tree/stmt.h>
#include <elash/ast/tree/expr.h>
#include <elash/ast/tree/type.h>
#include <elash/ast/tree/init.h>
#include <elash/ast/tree/toe.h>
#include <elash/ast/tree/unr.h>
#include <elash/ast/tree/toi.h>

#include <elash/source/span.h>
#include <elash/util/assert.h>
#include <elash/defs/sv.h>

#include <elash/unparser/unparser.h>
#include <elash/source/doc.h>

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

static ElAstType* gen_type(ElDynArena* arena, int depth);
static ElAstExpr* gen_expr(ElDynArena* arena, int depth);
static ElAstInit* gen_init(ElDynArena* arena, int depth);
static ElAstStmt* gen_stmt(ElDynArena* arena, int depth);
static ElAstDecl* gen_decl(ElDynArena* arena, int depth);
static ElAstToE*  gen_toe(ElDynArena* arena, int depth);
static ElAstToI*  gen_toi(ElDynArena* arena, int depth);
static ElAstExpr* gen_literal(ElDynArena* arena);

#define NSPAN EL_SRCSPAN_NULL

// NOLINTBEGIN(readability-magic-numbers)
// NOLINTBEGIN(bugprone-switch-missing-default-case)

static inline int nd(int depth) {
    return depth > 0 ? depth - 1 : 0;
}

static ElAstIdent* gen_ident(ElDynArena* arena) {
    char buf[32];
    snprintf(buf, sizeof(buf), "id%d", rand() % 1000);
    return el_ast_new_ident_raw(arena, NSPAN, el_dynarena_clone_sv(arena, el_sv_from_cstr(buf)));
}

static ElMutabilitySpec gen_mut() {
    return rand() % 3;
}

static ElAstType* gen_type(ElDynArena* arena, int depth) {
    if (depth <= 0) return el_ast_new_type_name(arena, NSPAN, gen_mut(), gen_ident(arena));
    switch (rand() % 6) {
    case 0: {
        usize count = (rand() % 3) + 1;
        ElAstDecl* fields = NULL;
        ElAstDecl* tail = NULL;
        for (usize i = 0; i < count; i++) el_ast_append_decl(&fields, &tail, gen_decl(arena, nd(depth)));
        return el_ast_new_type_struct(arena, NSPAN, gen_mut(), fields, count);
    }
    case 1: {
        usize count = (rand() % 3) + 1;
        ElAstType* head = NULL;
        ElAstType* tail = NULL;
        for (usize i = 0; i < count; i++) el_ast_type_list_append(&head, &tail, gen_type(arena, nd(depth)));
        return el_ast_new_type_tuple(arena, NSPAN, gen_mut(), head, count);
    }
    case 2: return el_ast_new_type_name(arena, NSPAN, gen_mut(), gen_ident(arena));
    case 3: return el_ast_new_type_ref(arena, NSPAN, gen_mut(), gen_type(arena, nd(depth)));
    case 4: return el_ast_new_type_array(arena, NSPAN, gen_mut(), gen_type(arena, nd(depth)), gen_expr(arena, nd(depth)));
    case 5: return el_ast_new_type_slice(arena, NSPAN, gen_mut(), gen_type(arena, nd(depth)), rand() % 2 == 0);
    }
    EL_UNREACHABLE("shouldn't get here");
}

static ElAstExpr* gen_literal(ElDynArena* arena) {
    switch (rand() % 6) {
    case 0: return el_ast_new_int_lit(arena, NSPAN, EL_INT128(rand() % 100));
    case 1: return el_ast_new_float_lit(arena, NSPAN, (double)(rand() % 100) / 10.0);
    case 2: return el_ast_new_char_lit(arena, NSPAN, (char)('a' + (rand() % 26)));
    case 3: return el_ast_new_str_lit(arena, NSPAN, el_sv_from_cstr("str"));
    case 4: return el_ast_new_bool_lit(arena, NSPAN, rand() % 2 == 0);
    case 5: return el_ast_new_null_lit(arena, NSPAN);
    }
    EL_UNREACHABLE("shouldn't get here");
}

static ElAstDeclarator* gen_declarators(ElDynArena* arena, int depth, bool allow_init) {
    usize count = (rand() % 3) + 1;

    ElAstDeclarator* head = NULL;
    ElAstDeclarator* tail = NULL;

    for (usize i = 0; i < count; i++)
        el_ast_append_declarator(&head, &tail, el_ast_new_declarator(
            arena, gen_ident(arena),
                allow_init && (rand() % 2 == 0) ? gen_init(arena, nd(depth)) : NULL));

    return head;
}

static ElAstInit* gen_brace_init(ElDynArena* arena, int depth) {
    if (depth <= 0) return el_ast_new_init_empty(arena, NSPAN);
    switch (rand() % 3) {
    case 0: return el_ast_new_init_empty(arena, NSPAN);
    case 1: return el_ast_new_init_list(arena, NSPAN, gen_init(arena, nd(depth)), 1);
    case 2: return el_ast_new_desig_init(arena, NSPAN, NULL, 0);
    }
    EL_UNREACHABLE("shouldn't get here");
}

static ElAstExpr* gen_expr(ElDynArena* arena, int depth) {
    if (depth <= 0) return gen_literal(arena);
    switch (rand() % 9) {
    case 0: return gen_literal(arena);
    case 1: return el_ast_new_ident(arena, NSPAN, gen_ident(arena)->name);
    case 2: return el_ast_new_bin_expr(arena, NSPAN, rand() % 14, gen_expr(arena, nd(depth)), gen_expr(arena, nd(depth)));
    case 3: return el_ast_new_unary_expr(arena, NSPAN, rand() % 8, gen_expr(arena, nd(depth)));
    case 4: return el_ast_new_typedinit(arena, NSPAN, EL_STORAGECLS_LOCAL, gen_type(arena, nd(depth)), gen_brace_init(arena, nd(depth)));
    case 5: return el_ast_new_call_expr(arena, NSPAN, gen_expr(arena, nd(depth)), gen_toi(arena, nd(depth)), 1);
    case 6: return el_ast_new_cast_expr(arena, NSPAN, rand() % 2, gen_expr(arena, nd(depth)), gen_type(arena, nd(depth)));
    case 7: return el_ast_new_member_expr(arena, NSPAN, gen_expr(arena, nd(depth)), gen_ident(arena)->name, rand() % 2 == 0);
    case 8: return el_ast_new_tmember_expr(arena, NSPAN, gen_expr(arena, nd(depth)), rand() % 5, NSPAN, rand() % 2 == 0);
    }
    EL_UNREACHABLE("shouldn't get here");
}

static ElAstInit* gen_init(ElDynArena* arena, int depth) {
    if (depth <= 0) return el_ast_new_init_expr(arena, gen_expr(arena, nd(depth)));
    switch (rand() % 3) {
    case 0: return el_ast_new_init_expr(arena, gen_expr(arena, nd(depth)));
    case 1: return el_ast_new_init_list(arena, NSPAN, gen_init(arena, nd(depth)), 1);
    case 2: return el_ast_new_desig_init(arena, NSPAN, NULL, 0);
    }
    EL_UNREACHABLE("shouldn't get here");
}

static ElBinOp gen_cassign_op(void) {
    static const ElBinOp ops[] = {
        EL_BIN_OP_ADD, EL_BIN_OP_SUB, EL_BIN_OP_MUL, EL_BIN_OP_DIV,
        EL_BIN_OP_MOD, EL_BIN_OP_BW_AND, EL_BIN_OP_BW_OR, EL_BIN_OP_BW_XOR,
        EL_BIN_OP_AND, EL_BIN_OP_OR, EL_BIN_OP_IMP, EL_BIN_OP_BW_IMP,
        EL_BIN_OP_SHL, EL_BIN_OP_SHR
    };
    return ops[rand() % (sizeof(ops) / sizeof(ops[0]))];
}

static ElAstDecl* gen_init_decl(ElDynArena* arena, int depth) {
    if (depth <= 0) return el_ast_new_var_decl(arena, NSPAN, gen_type(arena, depth + 1), gen_declarators(arena, depth, false));
    switch (rand() % 4) {
    case 0: return el_ast_new_alias(arena, NSPAN, gen_ident(arena)->name, *gen_toe(arena, nd(depth)));
    case 1: return el_ast_new_typedef(arena, NSPAN, gen_ident(arena)->name, gen_type(arena, nd(depth)));
    case 2: return el_ast_new_var_def(arena, NSPAN, gen_type(arena, nd(depth)), gen_declarators(arena, depth, true), rand() % 2 == 0);
    case 3: return el_ast_new_var_decl(arena, NSPAN, gen_type(arena, nd(depth)), gen_declarators(arena, depth, false));
    }
    EL_UNREACHABLE("shouldn't get here");
}

static ElAstStmt* gen_init_stmt(ElDynArena* arena, int depth) {
    switch (rand() % 4) {
    case 0: return el_ast_new_expr_stmt(arena, NSPAN, gen_expr(arena, nd(depth)));
    case 1: return el_ast_new_decl_stmt(arena, NSPAN, gen_init_decl(arena, nd(depth)));
    case 2: return el_ast_new_assign_stmt(arena, NSPAN, gen_expr(arena, nd(depth)), gen_init(arena, nd(depth)));
    case 3: return el_ast_new_compound_assign_stmt(arena, NSPAN, gen_cassign_op(), gen_expr(arena, nd(depth)), gen_init(arena, nd(depth)));
    }
    EL_UNREACHABLE("shouldn't get here");
}

static ElAstStmt* gen_stmt(ElDynArena* arena, int depth) {
    if (depth <= 0) return el_ast_new_expr_stmt(arena, NSPAN, gen_expr(arena, nd(depth)));
    switch (rand() % 10) {
    case 0: return el_ast_new_expr_stmt(arena, NSPAN, gen_expr(arena, nd(depth)));
    case 1: return el_ast_new_return_stmt(arena, NSPAN, gen_init(arena, nd(depth)));
    case 2: return el_ast_new_decl_stmt(arena, NSPAN, gen_decl(arena, nd(depth)));
    case 3: return el_ast_new_assign_stmt(arena, NSPAN, gen_expr(arena, nd(depth)), gen_init(arena, nd(depth)));
    case 4: return el_ast_new_block_stmt(arena, NSPAN, gen_stmt(arena, nd(depth)));
    case 5: return el_ast_new_compound_assign_stmt(arena, NSPAN, gen_cassign_op(), gen_expr(arena, nd(depth)), gen_init(arena, nd(depth)));
    case 6: return el_ast_new_if_stmt(arena, NSPAN, (rand() % 2 == 0) ? gen_init_stmt(arena, nd(depth)) : NULL, gen_expr(arena, nd(depth)), gen_stmt(arena, nd(depth)), NULL);
    case 7: return el_ast_new_while_stmt(arena, NSPAN, (rand() % 2 == 0) ? gen_init_stmt(arena, nd(depth)) : NULL, gen_expr(arena, nd(depth)), gen_stmt(arena, nd(depth)));
    case 8: return el_ast_new_break_stmt(arena, NSPAN);
    case 9: return el_ast_new_continue_stmt(arena, NSPAN);
    }
    EL_UNREACHABLE("shouldn't get here");
}

static ElAstStmt* gen_stmt_list(ElDynArena* arena, int depth) {
    uint count = (rand() % 5) + 1;
    ElAstStmt* head = NULL;
    ElAstStmt* tail = NULL;
    for (uint i = 0; i < count; i++) {
        el_ast_stmt_list_append(&head, &tail, gen_stmt(arena, nd(depth)));
    }
    return head;
}

static ElAstBlockStmt* gen_block(ElDynArena* arena, int depth) {
    return EL_DYNARENA_NEW_STRUCT(arena, ElAstBlockStmt, {
        .stmts = gen_stmt_list(arena, depth),
    });
}

static ElAstDecl* gen_decl(ElDynArena* arena, int depth) {
    if (depth <= 0) return el_ast_new_var_decl(arena, NSPAN, gen_type(arena, depth + 1), gen_declarators(arena, depth, false));
    switch (rand() % 6) {
        case 0: return el_ast_new_alias(arena, NSPAN, gen_ident(arena)->name, *gen_toe(arena, nd(depth)));
        case 1: return el_ast_new_typedef(arena, NSPAN, gen_ident(arena)->name, gen_type(arena, nd(depth)));
        case 2: return el_ast_new_var_def(arena, NSPAN, gen_type(arena, nd(depth)), gen_declarators(arena, depth, true), rand() % 2 == 0);
        case 3: return el_ast_new_var_decl(arena, NSPAN, gen_type(arena, nd(depth)), gen_declarators(arena, depth, false));
        case 4:
            return el_ast_new_func_def(arena, NSPAN, (ElAstFuncSignature) {
                NSPAN, gen_type(arena, nd(depth)), gen_ident(arena), el_ast_make_func_param_list()
            }, gen_block(arena, nd(depth)));
        case 5:
            return el_ast_new_func_decl(arena, NSPAN, (ElAstFuncSignature){
                NSPAN, gen_type(arena, nd(depth)), gen_ident(arena), el_ast_make_func_param_list()
            });
    }
    EL_UNREACHABLE("shouldn't get here");
}

static ElAstToE* gen_toe(ElDynArena* arena, int depth) {
    if (depth <= 0) return el_ast_new_toe_type(arena, gen_type(arena, nd(depth)));
    switch (rand() % 3) {
    case 0: return el_ast_new_toe_type(arena, gen_type(arena, nd(depth)));
    case 1: return el_ast_new_toe_expr(arena, gen_expr(arena, nd(depth)));
    case 2: return el_ast_new_toe_unr(arena, el_ast_new_unr_ident(arena, NSPAN, gen_ident(arena)));
    }
    EL_UNREACHABLE("shouldn't get here");
}

static ElAstToI* gen_toi(ElDynArena* arena, int depth) {
    if (depth <= 0) return el_ast_new_toi_init(arena, gen_init(arena, nd(depth)));
    switch (rand() % 3) {
    case 0: return el_ast_new_toi_type(arena, gen_type(arena, nd(depth)));
    case 1: return el_ast_new_toi_init(arena, gen_init(arena, nd(depth)));
    case 2: return el_ast_new_toi_unr(arena, el_ast_new_unr_ident(arena, NSPAN, gen_ident(arena)));
    }
    EL_UNREACHABLE("shouldn't get here");
}

ElAstModule* gen_module(ElDynArena* arena, int max_depth) {
    ElAstModule* module = el_ast_new_module(arena, NSPAN);
    uint num_decls = (rand() % 10) + 1;
    for (uint i = 0; i < num_decls; i++) {
        el_ast_module_append(module, gen_decl(arena, max_depth));
    }
    return module;
}

// NOLINTEND(bugprone-switch-missing-default-case)
// NOLINTEND(readability-magic-numbers)

#if defined(__unix__) || defined(__APPLE__) || defined(_POSIX_VERSION)
    #include <unistd.h>
    #define GETPID() getpid()
#elif defined(_WIN32)
    #include <process.h>
    #define GETPID() _getpid()
#else
    #define GETPID() 0
#endif

#define MAX_DEPTH 28
int main(int argc, char** argv) {
    ElDynArena arena;
    el_dynarena_init(&arena);

    if (argc > 1) {
        srand(atoi(argv[1]));
    } else {
        srand(time(NULL) + GETPID());
    }

    ElAstModule* mod = gen_module(&arena, MAX_DEPTH);

    ElTokenBuf tokens;
    el_tkbuf_init(&tokens);

    ElUnparser unparser;
    el_unparser_init(&unparser, &tokens, &arena);
    el_unparser_unparse_module(&unparser, mod);

    ElSourceDocument doc;
    el_srcdoc_init_empty(&doc, EL_SV("<generated>"));
    for (ElToken* tok = tokens.data; tok < tokens.data+tokens.len; ++tok) {
        el_srcdoc_append_token(&doc, tok);
    }

    el_srcdoc_print(&doc, stdout);

    el_tkbuf_destroy(&tokens);
    el_srcdoc_free(&doc);
    el_dynarena_free(&arena);
}
