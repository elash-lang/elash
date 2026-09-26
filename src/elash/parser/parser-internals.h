#pragma once
#include <elash/parser/parser.h> // IWYU pragma: export
#include <elash/util/assert.h>   // IWYU pragma: export

#include <elash/ast/tree/toe.h>
#include <elash/ast/tree/unr.h>
#include <elash/source/span.h>

void _el_parser_report_expected(ElParser* parser, ElTokenType expected);
void _el_parser_report_expected_at(ElParser* parser, ElTokenType expected, usize idx);
void _el_parser_report_unexpected(ElParser* parser, ElToken tok);

bool _el_parser_lookahead_skip_type(ElParser* parser, usize* idx);
bool _el_parser_is_type_literal(ElParser* parser);

ElAstStmt*  _el_parse_block(ElParser* parser, ElToken lbrace_tok);
ElAstIdent* _el_parse_ident(ElParser* parser);
ElAstType*  _el_parse_type(ElParser* parser);
ElAstType*  _el_parse_type_suffixes(ElParser* parser, ElAstType* type);
ElAstType*  _el_parse_type_mut_and_suffixes(ElParser* parser, ElAstType* type);

ElAstExpr* _el_parse_primary(ElParser* parser);
ElAstExpr* _el_parse_postfix(ElParser* parser);
ElAstExpr* _el_parse_continue_postfixes(ElParser* parser, ElAstExpr* expr);
ElAstExpr* _el_parse_call(ElParser* parser, ElAstExpr* callee);
ElAstExpr* _el_parse_member(ElParser* parser, ElAstExpr* expr, bool is_optional);

bool _el_parse_const_idx(ElParser* parser, ElToken tok, usize* out);

typedef enum ElParseAmbigKind {
    EL_PARSE_AMBIG_TYPE,
    EL_PARSE_AMBIG_EXPR,
    EL_PARSE_AMBIG_UNR,
} ElParseAmbigKind;

typedef struct ElParseAmbig {
    ElParseAmbigKind kind;
    ElSourceSpan     span;
    union {
        ElAstType* type;
        ElAstExpr* expr;
        ElAstUnr*  unr;
    } as;
} ElParseAmbig;

bool         _el_parser_is_complex_expr(ElParser* parser);
ElParseAmbig _el_parse_ambig(ElParser* parser);
ElAstToE*    _el_parser_toe_from_ambig(ElParser* parser, ElParseAmbig node);

/////////////// mutability specifiers ////////////////
static inline bool is_mut_spec_token(ElTokenType type) {
    return type == EL_TT_KW_CONST || type == EL_TT_KW_WONLY;
}

static inline ElMutabilitySpec mut_spec_from_token(ElTokenType type) {
    if (type == EL_TT_KW_CONST) return EL_MUTSPEC_CONST;
    if (type == EL_TT_KW_WONLY) return EL_MUTSPEC_WONLY;
    EL_UNREACHABLE("invalid argument passed");
}
