#pragma once
#include <elash/pp/preproc.h>  // IWYU pragma: export
#include <elash/util/assert.h> // IWYU pragma: export
#include <elash/util/todo.h>   // IWYU pragma: export

#include <elash/sema/unary-op.h>
#include <elash/sema/bin-op.h>

#include <elash/lexer/tokarr.h>
#include <elash/util/int128.h>

//////// operation count /////////
#define EL_PP_DIR_OPS  4
#define EL_PP_EXPR_OPS 1
#define EL_PP_ITER_OPS 8

bool _el_pp_ensure_ops_available(ElPreproc* pp, ElSourceSpan span);

//////// include frames ////////
#define INCLUDE_DEPTH_LIMIT 220

typedef enum FrameType {
    FRAME_CALL,
    FRAME_INCLUDE,
    FRAME_LOOP_BODY,
} FrameType;

typedef struct ElPpFrame {
    ElTokenStream           stream;
    const ElSourceDocument* doc;
    struct ElPpFrame*       parent;
    ElToken                 pushback;
    bool                    has_pushback;
    FrameType               type;
} ElPpFrame;

void _el_pp_push_frame(ElPreproc* pp, ElTokenStream stream, const ElSourceDocument* doc);
void _el_pp_pop_frame(ElPreproc* pp);

void _el_pp_push_while_body_frame(ElPreproc* pp, ElPpFrame* frame, ElTokenStream stream);
void _el_pp_push_eval_frame(ElPreproc* pp, ElTokenStream stream);

///////////// blocks //////////////
typedef enum ElPpBlockKind {
    EL_PP_BLOCK_IF,
    EL_PP_BLOCK_FUNC,
    EL_PP_BLOCK_WHILE,
    EL_PP_BLOCK_FOR,
} ElPpBlockKind;

typedef struct ElPpIfState {
    bool branch_taken;
    bool has_scope;
    bool had_else;
} ElPpIfState;

typedef struct ElPpFuncState {
    bool is_public;
    ElStringView name;
    ElPpParamList params;
} ElPpFuncState;

typedef enum ElPpLoopKind {
    EL_PP_LOOP_WHILE,
    EL_PP_LOOP_FOR,
} ElPpLoopKind;

typedef struct ElPpLoopState {
    ElPpLoopKind kind;
    ElTokenArray body;
    ElTokenArrayStream body_stream;
    ElPpFrame body_frame;
    bool capturing_body;
    union {
        struct {
            ElTokenArray cond;
        } while_;
        struct {
            ElStringView name;
            ElPpValue* iterable;
            usize index;
        } for_;
    } as;
} ElPpLoopState;

struct ElPpBlock {
    ElPpBlockKind kind;
    ElSourceSpan  open_span;
    ElPpBlock*    parent;

    union {
        ElPpIfState   if_;
        ElPpFuncState func;
        ElPpLoopState loop;
    } as;
};

ElPpBlock* _el_pp_push_block(ElPreproc* pp, ElPpBlockKind kind, ElSourceSpan span);
void _el_pp_pop_block(ElPreproc* pp);

void _el_pp_push_if_block(ElPreproc* pp, bool take_branch, ElSourceSpan ifspan);
void _el_pp_push_while_block(ElPreproc* pp, ElSourceSpan whilespan, ElTokenArray cond, bool cond_val);
void _el_pp_push_for_block(ElPreproc* pp, ElSourceSpan forspan, ElStringView name, ElPpValue* iterable, bool has_items);
void _el_pp_push_func_block(ElPreproc* pp, ElSourceSpan defspan, bool is_public, ElStringView name, ElPpParamList params);

void _el_pp_enter_if_branch(ElPreproc* pp);
void _el_pp_leave_if_branch(ElPreproc* pp);
void _el_pp_pop_if_block(ElPreproc* pp);

ElStringView _el_pp_block_kind_name(ElPpBlockKind kind);

////////// call frames /////////
#define CALL_DEPTH_LIMIT 80

struct ElPpCallFrame {
    ElPpValue* return_value;
    bool       has_returned;

    ElSourceSpan call_span;
    ElPpSymbol*  func;

    ElPpBlock* saved_block_stack;
    uint       saved_skip_depth;

    ElPpFrame* body_frame;
    ElPpFrame* caller_frame;

    ElPpCallFrame* parent;
};

////////// scopes //////////
ElPpScope* _el_pp_push_scope(ElPreproc* pp);
ElPpScope* _el_pp_pop_scope(ElPreproc* pp);

////////// directives //////////
bool _el_pp_preprocess_directive(ElPreproc* pp, ElToken hash, ElToken* out_tok);
bool _el_pp_skip_directive(ElPreproc* pp, ElToken hash);

bool _el_pp_handle_include(ElPreproc* pp, ElSourceSpan dspan);
bool _el_pp_handle_embed(ElPreproc* pp, ElSourceSpan dspan);

bool _el_pp_skip_include(ElPreproc* pp);
bool _el_pp_skip_embed(ElPreproc* pp);

bool _el_pp_handle_const(ElPreproc* pp, ElSourceSpan dspan);
bool _el_pp_handle_var(ElPreproc* pp, ElSourceSpan dspan);
bool _el_pp_handle_set(ElPreproc* pp, ElSourceSpan dspan);
bool _el_pp_handle_inc(ElPreproc* pp, ElSourceSpan dspan);
bool _el_pp_handle_dec(ElPreproc* pp, ElSourceSpan dspan);

bool _el_pp_skip_const(ElPreproc* pp);
bool _el_pp_skip_var(ElPreproc* pp);
bool _el_pp_skip_set(ElPreproc* pp);
bool _el_pp_skip_incdec(ElPreproc* pp);

bool _el_pp_handle_diag(ElPreproc* pp, ElDiagSeverity sev, ElSourceSpan span);
bool _el_pp_skip_diag(ElPreproc* pp);

bool _el_pp_handle_emit(ElPreproc* pp, ElSourceSpan dspan);
bool _el_pp_skip_emit(ElPreproc* pp);

bool _el_pp_handle_if(ElPreproc* pp, ElSourceSpan dspan);
bool _el_pp_handle_else(ElPreproc* pp, ElSourceSpan dspan);
bool _el_pp_handle_elif(ElPreproc* pp, ElSourceSpan dspan);
bool _el_pp_handle_end(ElPreproc* pp, ElSourceSpan dspan);

bool _el_pp_skip_if(ElPreproc* pp);
bool _el_pp_skip_else(ElPreproc* pp, ElSourceSpan dspan);
bool _el_pp_skip_elif(ElPreproc* pp, ElSourceSpan dspan);
bool _el_pp_skip_end(ElPreproc* pp);

bool _el_pp_handle_func(ElPreproc* pp, ElSourceSpan dspan);
bool _el_pp_handle_return(ElPreproc* pp, ElSourceSpan dspan);
bool _el_pp_skip_func(ElPreproc* pp);
bool _el_pp_skip_return(ElPreproc* pp);

bool _el_pp_handle_while(ElPreproc* pp, ElSourceSpan dspan);
bool _el_pp_handle_for(ElPreproc* pp, ElSourceSpan dspan);
bool _el_pp_skip_while(ElPreproc* pp);
bool _el_pp_skip_for(ElPreproc* pp);

bool _el_pp_handle_continue(ElPreproc* pp, ElSourceSpan dspan);
bool _el_pp_handle_break(ElPreproc* pp, ElSourceSpan dspan);
bool _el_pp_skip_continue(ElPreproc* pp);
bool _el_pp_skip_break(ElPreproc* pp);

bool _el_pp_finish_loop(ElPreproc* pp);
bool _el_pp_loop_body_exhausted(ElPreproc* pp);

/////////// functions ////////////
typedef struct ElPpArgList {
    ElPpValue* head;
    ElPpValue* tail;
    usize count;
} ElPpArgList;

void _el_pp_append_param(ElPpParamList* list, ElDynArena* arena, ElStringView name);
void _el_pp_append_arg(ElPpArgList* list,  ElPpValue* val);

bool _el_pp_finish_pending_func(ElPreproc* pp);
ElPpValue* _el_pp_call_func(ElPreproc* pp, ElPpSymbol* sym, ElSourceSpan cspan);

////////// expressions ///////////
typedef enum ElPpNumKind {
    EL_PP_NUM_INT,
    EL_PP_NUM_FLOAT,
} ElPpNumKind;

typedef struct ElPpNum {
    ElPpNumKind kind;
    union {
        ElInt128 int_;
        double   float_;
    } as;
} ElPpNum;

bool _el_pp_to_num(ElPpValue* val, ElPpNum* out);
bool _el_pp_to_int(ElPpValue* val, int64_t* out);

ElPpValue* _el_pp_apply_numeric_bin(
    ElPreproc* pp, ElSourceSpan span, ElBinOp op, ElPpNum lhs, ElPpNum rhs
);
ElPpValue* _el_pp_apply_bin_op(
    ElPreproc* pp, ElSourceSpan span, ElBinOp op, ElPpValue* lhs, ElPpValue* rhs
);
ElPpValue* _el_pp_apply_unary_op(
    ElPreproc* pp, ElSourceSpan span, ElUnaryOp op, ElPpValue* operand
);

ElPpValue* _el_pp_eval(ElPreproc* pp);
bool _el_pp_skip_expr(ElPreproc* pp);

////////// tokens ///////////
bool _el_pp_next(ElPreproc* pp, ElToken* out_tok);
bool _el_pp_peek(ElPreproc* pp, ElToken* out_tok);
bool _el_pp_read(ElPreproc* pp, ElToken* out_tok);

bool _el_pp_next_d(ElPreproc* pp, ElToken* out_tok);

/////////// parsing helpers //////////
ElToken _el_pp_advance(ElPreproc* pp);

bool _el_pp_match(ElPreproc* pp, ElTokenType type);
bool _el_pp_expect(ElPreproc* pp, ElTokenType type);

////////// utilities /////////////
bool _el_pp_ensure_bool(ElPreproc* pp, ElPpValue* val, ElSourceSpan dspan, ElStringView dname);

/////// diagnostics ////////////
void* _el_pp_report_deref(ElPreproc* pp, ElSourceSpan span);
void* _el_pp_report_incdec(ElPreproc* pp, ElSourceSpan span);
void* _el_pp_report_unterm_quote(ElPreproc* pp, ElSourceSpan span);
void* _el_pp_report_float_bw(ElPreproc* pp, ElSourceSpan span, ElBinOp op);
void* _el_pp_report_non_bool_logical(ElPreproc* pp, ElSourceSpan span, ElBinOp op);
void* _el_pp_report_non_bool_logical_unary(ElPreproc* pp, ElSourceSpan span, ElUnaryOp op);
