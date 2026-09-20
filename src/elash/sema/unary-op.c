#include <elash/sema/unary-op.h>

#include <elash/util/assert.h>
#include <elash/defs/sv.h>

ElStringView el_unary_op_to_string(ElUnaryOp type) {
    switch (type) {
    case EL_UNARY_OP_POS: return EL_SV("+");
    case EL_UNARY_OP_NEG: return EL_SV("-");

    case EL_UNARY_OP_NOT:    return EL_SV("!");
    case EL_UNARY_OP_BW_NOT: return EL_SV("~");

    case EL_UNARY_OP_ADDROF: return EL_SV("&");
    case EL_UNARY_OP_DEREF: return EL_SV("^");

    case EL_UNARY_OP_PRE_INC:
    case EL_UNARY_OP_POST_INC: return EL_SV("++");

    case EL_UNARY_OP_PRE_DEC:
    case EL_UNARY_OP_POST_DEC: return EL_SV("--");

    case EL_UNARY_OP_OPT_UNWRAP: return EL_SV("!");
    }
    EL_UNREACHABLE_ENUM_VAL(ElUnaryOp, type);
}

ElStringView el_unary_op_format(ElUnaryOp type) {
    switch (type) {
    case EL_UNARY_OP_POS: return EL_SV("+");
    case EL_UNARY_OP_NEG: return EL_SV("-");

    case EL_UNARY_OP_NOT:    return EL_SV("!");
    case EL_UNARY_OP_BW_NOT: return EL_SV("~");

    case EL_UNARY_OP_ADDROF: return EL_SV("&");
    case EL_UNARY_OP_DEREF: return EL_SV("^");

    case EL_UNARY_OP_PRE_INC:  return EL_SV("pre  ++x");
    case EL_UNARY_OP_PRE_DEC:  return EL_SV("pre  --x");
    case EL_UNARY_OP_POST_INC: return EL_SV("post x++");
    case EL_UNARY_OP_POST_DEC: return EL_SV("post x--");
    case EL_UNARY_OP_OPT_UNWRAP: return EL_SV("post x!");
    }
    EL_UNREACHABLE_ENUM_VAL(ElUnaryOp, type);
}
