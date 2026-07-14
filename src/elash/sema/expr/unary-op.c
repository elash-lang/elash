#include <elash/sema/expr/unary-op.h>

#include <elash/util/assert.h>
#include <elash/defs/sv.h>

ElStringView el_sema_unary_op_to_string(ElSemaUnaryOp type) {
    switch (type) {
    case EL_SEMA_UNARY_OP_POS: return EL_SV("+");
    case EL_SEMA_UNARY_OP_NEG: return EL_SV("-");

    case EL_SEMA_UNARY_OP_NOT:    return EL_SV("!");
    case EL_SEMA_UNARY_OP_BW_NOT: return EL_SV("~");

    case EL_SEMA_UNARY_OP_ADDROF: return EL_SV("&");
    case EL_SEMA_UNARY_OP_DEREF: return EL_SV("^");

    case EL_SEMA_UNARY_OP_PRE_INC:
    case EL_SEMA_UNARY_OP_POST_INC: return EL_SV("++");

    case EL_SEMA_UNARY_OP_PRE_DEC:
    case EL_SEMA_UNARY_OP_POST_DEC: return EL_SV("--");
    }
    EL_UNREACHABLE_ENUM_VAL(ElSemaUnaryOp, type);
}

ElStringView el_sema_unary_op_format(ElSemaUnaryOp type) {
    switch (type) {
    case EL_SEMA_UNARY_OP_POS: return EL_SV("+");
    case EL_SEMA_UNARY_OP_NEG: return EL_SV("-");

    case EL_SEMA_UNARY_OP_NOT:    return EL_SV("!");
    case EL_SEMA_UNARY_OP_BW_NOT: return EL_SV("~");

    case EL_SEMA_UNARY_OP_ADDROF: return EL_SV("&");
    case EL_SEMA_UNARY_OP_DEREF: return EL_SV("^");

    case EL_SEMA_UNARY_OP_PRE_INC:  return EL_SV("pre  ++x");
    case EL_SEMA_UNARY_OP_PRE_DEC:  return EL_SV("pre  --x");
    case EL_SEMA_UNARY_OP_POST_INC: return EL_SV("post x++");
    case EL_SEMA_UNARY_OP_POST_DEC: return EL_SV("post x--");
    }
    EL_UNREACHABLE_ENUM_VAL(ElSemaUnaryOp, type);
}

