#pragma once
#include <elash/pp/preproc.h> // IWYU pragma: export
#include <elash/util/assert.h> // IWYU pragma: export

#include <elash/sema/unary-op.h>
#include <elash/sema/bin-op.h>

//////// include frames ////////
#define INCLUDE_DEPTH_LIMIT 220

typedef struct ElPpFrame {
    ElTokenStream           stream;
    const ElSourceDocument* doc;
    struct ElPpFrame*       parent;
    ElToken                 pushback;
    bool                    has_pushback;
} ElPpFrame;

void _el_pp_push_frame(ElPreproc* pp, ElTokenStream stream, const ElSourceDocument* doc);

////////// if frames //////////
typedef struct ElPpIfFrame ElPpIfFrame;
struct ElPpIfFrame {
    ElSourceSpan ifspan;
    bool branch_taken;
    bool has_scope;
    bool had_else;
    ElPpIfFrame* parent;
};

void _el_pp_push_if_frame(ElPreproc* pp, bool take_branch, ElSourceSpan ifspan);
void _el_pp_enter_if_branch(ElPreproc* pp);
void _el_pp_leave_if_branch(ElPreproc* pp);
void _el_pp_pop_if_frame(ElPreproc* pp);

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

////////// expressions ///////////
typedef enum ElPpNumKind {
    EL_PP_NUM_INT,
    EL_PP_NUM_FLOAT,
} ElPpNumKind;

typedef struct ElPpNum {
    ElPpNumKind kind;
    union {
        int64_t int_;
        double  float_;
    } as;
} ElPpNum;

bool _el_pp_to_num(ElPpValue* val, ElPpNum* out);
bool _el_pp_to_int(ElPpValue* val, int64_t* out);

ElPpValue* _el_pp_apply_numeric_bin(
    ElPreproc* pp, ElSourceSpan span, ElSemaBinOp op, ElPpNum lhs, ElPpNum rhs
);
ElPpValue* _el_pp_apply_bin_op(
    ElPreproc* pp, ElSourceSpan span, ElSemaBinOp op, ElPpValue* lhs, ElPpValue* rhs
);
ElPpValue* _el_pp_apply_unary_op(
    ElPreproc* pp, ElSourceSpan span, ElSemaUnaryOp op, ElPpValue* operand
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

/////// diagnostics ///////
void* _el_pp_report_deref(ElPreproc* pp, ElSourceSpan span);
void* _el_pp_report_incdec(ElPreproc* pp, ElSourceSpan span);
void* _el_pp_report_unterm_quote(ElPreproc* pp, ElSourceSpan span);
void* _el_pp_report_float_bw(ElPreproc* pp, ElSourceSpan span, ElSemaBinOp op);
void* _el_pp_report_non_bool_logical(ElPreproc* pp, ElSourceSpan span, ElSemaBinOp op);
void* _el_pp_report_non_bool_logical_unary(ElPreproc* pp, ElSourceSpan span, ElSemaUnaryOp op);
