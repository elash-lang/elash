#pragma once

#include <elash/defs/sv.h>

typedef enum ElBinOp {
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

    EL_SEMA_BIN_OP_OPT_FB,   // ??
    EL_SEMA_BIN_OP_OPT_MAP,  // ?>

    EL_SEMA_BIN_OP_BW_AND, // &
    EL_SEMA_BIN_OP_BW_OR,  // |
    EL_SEMA_BIN_OP_BW_XOR, // <>
    EL_SEMA_BIN_OP_BW_IMP, // ~>
    EL_SEMA_BIN_OP_SHL,    // <<
    EL_SEMA_BIN_OP_SHR,    // >>

    EL_SEMA_BIN_OP_INDEX,  // []
} ElBinOp;

ElStringView el_bin_op_to_string(ElBinOp type);

static inline bool el_bin_op_is_optional(ElBinOp op) {
    return op == EL_SEMA_BIN_OP_OPT_FB || op == EL_SEMA_BIN_OP_OPT_MAP;
}
static inline bool el_bin_op_is_arithmetic(ElBinOp op) {
    return op >= EL_SEMA_BIN_OP_ADD && op <= EL_SEMA_BIN_OP_MOD;
}
static inline bool el_bin_op_is_comparison(ElBinOp op) {
    return op >= EL_SEMA_BIN_OP_EQ && op <= EL_SEMA_BIN_OP_GTE;
}
static inline bool el_bin_op_is_equality(ElBinOp op) {
    return op == EL_SEMA_BIN_OP_EQ || op == EL_SEMA_BIN_OP_NEQ;
}
static inline bool el_bin_op_is_logical(ElBinOp op) {
    return op == EL_SEMA_BIN_OP_AND || op == EL_SEMA_BIN_OP_OR || op == EL_SEMA_BIN_OP_IMP;
}
static inline bool el_bin_op_is_bitwise(ElBinOp op) {
    return op >= EL_SEMA_BIN_OP_BW_AND && op <= EL_SEMA_BIN_OP_SHR;
}

