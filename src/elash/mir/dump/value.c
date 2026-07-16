#include <elash/mir/dump/value.h>
#include <elash/mir/symbol.h>
#include <elash/mir/type.h>
#include <elash/util/assert.h>

#include <inttypes.h>

void el_mir_dump_value(const ElMirValue* value, FILE* out) {
    switch (value->kind) {
    case EL_MIR_VAL_REG:
        fprintf(out, "%%r%"PRIu32, value->as.reg.id);
        break;
    case EL_MIR_VAL_ARG:
        fprintf(out, "%%a%"PRIu32, value->as.arg.idx);
        break;
    case EL_MIR_VAL_CONST:
        if (value->type->kind == EL_MIR_TYPE_INT) {
            if (value->type->as.integer.width == 1) {
                fputs(value->as.constant.as.int_ ? "true" : "false", out);
            } else {
                fprintf(out, "$%"PRId64, value->as.constant.as.int_);
            }
        } else {
            fputs("<unhandled const>", out);
        }
        break;
    case EL_MIR_VAL_GLOBAL:
        el_mir_dump_symbol(value->as.global.sym, out);
        break;
    }

    fputs(":", out);
    el_mir_dump_type(value->type, out);
}
