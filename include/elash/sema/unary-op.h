#pragma once

#include <elash/defs/sv.h>

typedef enum ElUnaryOp {
    EL_UNARY_OP_POS, // +
    EL_UNARY_OP_NEG, // -

    EL_UNARY_OP_NOT,    // !
    EL_UNARY_OP_BW_NOT, // ~

    EL_UNARY_OP_DEREF,  // ^
    EL_UNARY_OP_ADDROF, // &

    EL_UNARY_OP_PRE_INC,  // ++x
    EL_UNARY_OP_PRE_DEC,  // --x
    EL_UNARY_OP_POST_INC, // x++
    EL_UNARY_OP_POST_DEC, // x--

    EL_UNARY_OP_OPT_UNWRAP, // x!
} ElUnaryOp;

static inline bool el_unary_op_is_post(ElUnaryOp op) {
    return op == EL_UNARY_OP_POST_INC
        || op == EL_UNARY_OP_POST_DEC
        || op == EL_UNARY_OP_OPT_UNWRAP;
}

static inline bool el_unary_op_is_incdec(ElUnaryOp op) {
    return op >= EL_UNARY_OP_PRE_INC
        && op <= EL_UNARY_OP_POST_DEC;
}

ElStringView el_unary_op_format(ElUnaryOp type);
ElStringView el_unary_op_to_string(ElUnaryOp type);
