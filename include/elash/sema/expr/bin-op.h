#pragma once

#include <elash/defs/sv.h>

typedef enum ElSemaBinOp {
    EL_SEMA_BIN_OP_ADD, // +
    EL_SEMA_BIN_OP_SUB, // -
    EL_SEMA_BIN_OP_MUL, // *
    EL_SEMA_BIN_OP_DIV, // /
    EL_SEMA_BIN_OP_MOD, // %

    EL_SEMA_BIN_OP_EQ,  // ==
    EL_SEMA_BIN_OP_NEQ, // !=
    EL_SEMA_BIN_OP_LT,  // <
    EL_SEMA_BIN_OP_LTE, // <=
    EL_SEMA_BIN_OP_GT,  // >
    EL_SEMA_BIN_OP_GTE, // >=
    
    EL_SEMA_BIN_OP_AND, // &&
    EL_SEMA_BIN_OP_OR,  // ||
    EL_SEMA_BIN_OP_IMP, // =>

    EL_SEMA_BIN_OP_BW_AND, // &
    EL_SEMA_BIN_OP_BW_OR,  // |
    EL_SEMA_BIN_OP_BW_XOR, // ^
    EL_SEMA_BIN_OP_BW_IMP, // ~>
    EL_SEMA_BIN_OP_SHL,    // <<
    EL_SEMA_BIN_OP_SHR,    // >>

    EL_SEMA_BIN_OP_INDEX,  // []
} ElSemaBinOp;

ElStringView el_sema_bin_op_to_string(ElSemaBinOp type);

static inline bool el_sema_bin_op_is_arithmetic(ElSemaBinOp op) {
    return op >= EL_SEMA_BIN_OP_ADD && op <= EL_SEMA_BIN_OP_MOD;
}
static inline bool el_sema_bin_op_is_comparison(ElSemaBinOp op) {
    return op >= EL_SEMA_BIN_OP_EQ && op <= EL_SEMA_BIN_OP_GTE;
}
static inline bool el_sema_bin_op_is_logical(ElSemaBinOp op) {
    return op == EL_SEMA_BIN_OP_AND || op == EL_SEMA_BIN_OP_OR || op == EL_SEMA_BIN_OP_IMP;
}
static inline bool el_sema_bin_op_is_bitwise(ElSemaBinOp op) {
    return op >= EL_SEMA_BIN_OP_BW_AND && op <= EL_SEMA_BIN_OP_SHR;
}

