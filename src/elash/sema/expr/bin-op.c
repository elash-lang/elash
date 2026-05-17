#include <elash/sema/expr/bin-op.h>

#include <elash/util/assert.h>
#include <elash/defs/sv.h>

ElStringView el_sema_bin_op_to_string(ElSemaBinOp type) {
    switch (type) {
    case EL_SEMA_BIN_OP_ADD: return EL_SV("+");
    case EL_SEMA_BIN_OP_SUB: return EL_SV("-");
    case EL_SEMA_BIN_OP_MUL: return EL_SV("*");
    case EL_SEMA_BIN_OP_DIV: return EL_SV("/");
    case EL_SEMA_BIN_OP_MOD: return EL_SV("%");

    case EL_SEMA_BIN_OP_EQ : return EL_SV("==");
    case EL_SEMA_BIN_OP_NEQ: return EL_SV("!=");
    case EL_SEMA_BIN_OP_LT : return EL_SV("<");
    case EL_SEMA_BIN_OP_LTE: return EL_SV("<=");
    case EL_SEMA_BIN_OP_GT : return EL_SV(">");
    case EL_SEMA_BIN_OP_GTE: return EL_SV(">=");
    
    case EL_SEMA_BIN_OP_AND: return EL_SV("&&");
    case EL_SEMA_BIN_OP_OR : return EL_SV("||");
    
    case EL_SEMA_BIN_OP_BW_AND: return EL_SV("&");
    case EL_SEMA_BIN_OP_BW_OR:  return EL_SV("|");
    case EL_SEMA_BIN_OP_BW_XOR: return EL_SV("^");
    case EL_SEMA_BIN_OP_SHL:    return EL_SV("<<");
    case EL_SEMA_BIN_OP_SHR:    return EL_SV(">>");

    case EL_SEMA_BIN_OP_INDEX:  return EL_SV("[]");
    }
    EL_UNREACHABLE_ENUM_VAL(ElSemaBinOp, type);
}
