#include <elash/mir/dump/value.h>
#include <elash/mir/symbol.h>
#include <elash/mir/type.h>
#include <elash/util/assert.h>

#include <inttypes.h>

void el_mir_dump_value(const ElMirValue* value, ElWriter* out) {
    switch (value->kind) {
    case EL_MIR_VAL_REG:
        el_writef(out, "%%r%"PRIu32, value->as.reg.id);
        break;
    case EL_MIR_VAL_ARG:
        el_writef(out, "%%a%"PRIu32, value->as.arg.idx);
        break;
    case EL_MIR_VAL_CONST:
        if (value->type->kind == EL_MIR_TYPE_INT) {
            if (value->type->as.integer.width == 1) {
                el_writef(out, "%s", el_i128_eq(value->as.constant.as.int_, EL_INT128(0)) ? "true" : "false");
            } else {
                // NOLINTBEGIN(readability-magic-numbers): A full commitment's what I'm thinking of
                el_write(out, el_i128_to_string(value->as.constant.as.int_, 10, (char[42]){}));
                // NOLINTEND(readability-magic-numbers)
            }
        } else {
            el_swrite(out, "<unhandled const>");
        }
        break;
    case EL_MIR_VAL_GLOBAL:
        el_mir_dump_symbol(value->as.global.sym, out);
        break;
    }

    el_write_char(out, ':');
    el_mir_dump_type(value->type, out);
}
