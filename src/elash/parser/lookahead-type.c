#include "parser-internals.h"

#include <elash/ast/tree/type.h>

static bool lookahead_skip_balanced(ElParser* parser, usize* idx) {
    uint depth_paren = 0;
    uint depth_brace = 0;
    uint depth_bracket = 0;

    do {
        ElToken tok = el_parser_peek_at(parser, (*idx)++);
        if (tok.type == EL_TT_EOF) return false;

        if (tok.type == EL_TT_LPAREN)        { depth_paren++;                          }
        else if (tok.type == EL_TT_RPAREN)   { if (depth_paren > 0) depth_paren--;     }
        else if (tok.type == EL_TT_LBRACE)   { depth_brace++;                          }
        else if (tok.type == EL_TT_RBRACE)   { if (depth_brace > 0) depth_brace--;     }
        else if (tok.type == EL_TT_LBRACKET) { depth_bracket++;                        }
        else if (tok.type == EL_TT_RBRACKET) { if (depth_bracket > 0) depth_bracket--; }

        if (depth_paren == 0 && depth_brace == 0 && depth_bracket == 0) break;
    } while (true);

    return true;
}

static bool lookahead_skip_type_base(ElParser* parser, usize* idx) {
    ElToken tok = el_parser_peek_at(parser, *idx);
    if (is_mut_spec_token(tok.type)) {
        (*idx)++;
        tok = el_parser_peek_at(parser, *idx);
    }

    if (tok.type == EL_TT_IDENT) {
        (*idx)++;
        ElToken next = el_parser_peek_at(parser, *idx);
        if (is_mut_spec_token(next.type)) (*idx)++;
        return true;
    }

    if (tok.type != EL_TT_KW_STRUCT)
        return false;

    (*idx)++;

    tok = el_parser_peek_at(parser, *idx);
    if (tok.type == EL_TT_LPAREN || tok.type == EL_TT_LBRACE) {
        if (!lookahead_skip_balanced(parser, idx)) return false;
        ElToken next = el_parser_peek_at(parser, *idx);
        if (is_mut_spec_token(next.type)) (*idx)++;
        return true;
    }

    return false;
}

static bool lookahead_skip_type_suffixes(ElParser* parser, usize* idx) {
    while (true) {
        ElToken tok = el_parser_peek_at(parser, *idx);

        if (tok.type == EL_TT_BITWISE_AND || tok.type == EL_TT_OPT) {
            (*idx)++;
            ElToken next = el_parser_peek_at(parser, *idx);
            if (is_mut_spec_token(next.type)) (*idx)++;
        } else if (tok.type == EL_TT_LBRACKET) {
            if (!lookahead_skip_balanced(parser, idx))
                return false;
            ElToken next = el_parser_peek_at(parser, *idx);
            if (is_mut_spec_token(next.type)) (*idx)++;
        } else {
            break;
        }
    }

    return true;
}

bool _el_parser_lookahead_skip_type(ElParser* parser, usize* idx) {
    return lookahead_skip_type_base(parser, idx)
        && lookahead_skip_type_suffixes(parser, idx);
}
