#pragma once

#include <elash/defs/sv.h>

typedef enum ElSemaUnaryOp {
    EL_SEMA_UNARY_OP_POS, // +
    EL_SEMA_UNARY_OP_NEG, // -

    EL_SEMA_UNARY_OP_NOT,    // !
    EL_SEMA_UNARY_OP_BW_NOT, // ~

    EL_SEMA_UNARY_OP_DEREF,  // ^
    EL_SEMA_UNARY_OP_ADDROF, // &

    EL_SEMA_UNARY_OP_PRE_INC,  // ++x
    EL_SEMA_UNARY_OP_PRE_DEC,  // --x
    EL_SEMA_UNARY_OP_POST_INC, // x++
    EL_SEMA_UNARY_OP_POST_DEC, // x--
} ElSemaUnaryOp;

static inline bool el_sema_unary_op_is_post(ElSemaUnaryOp op) {
    return op == EL_SEMA_UNARY_OP_POST_INC || op == EL_SEMA_UNARY_OP_POST_DEC;
}

ElStringView el_sema_unary_op_format(ElSemaUnaryOp type);
ElStringView el_sema_unary_op_to_string(ElSemaUnaryOp type);
