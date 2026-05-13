#include <elash/mir/dump/value.h>
#include <elash/sema/symbol/dump.h>
#include <elash/sema/type.h>
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
        if (value->type->kind == EL_TYPE_PRIM) {
            switch (value->type->as.prim.kind) {
            case EL_PRIMTYPE_INT:   fprintf(out, "$%"PRId64, value->as.constant.lit.as.int_);       break;
            case EL_PRIMTYPE_UINT:  fprintf(out, "$%"PRIu64, value->as.constant.lit.as.uint_);      break;
            case EL_PRIMTYPE_CHAR:  fprintf(out, "$'%c'", value->as.constant.lit.as.char_);         break;
            case EL_PRIMTYPE_BOOL:  fputs(value->as.constant.lit.as.bool_ ? "true" : "false", out); break;
            case EL_PRIMTYPE_VOID:  EL_UNREACHABLE("void constant");                                break;
            }
        } else {
            fputs("<unhandled const>", out);
        }
        break;
    case EL_MIR_VAL_GLOBAL:
        el_sema_dump_symbol(value->as.global.sym, out);
        break;
    }

    fputs(":", out);
    el_sema_dump_type(value->type, out);
}
