#include <elash/mir/instr/gep.h>
#include <elc/codegen/builtin/llvm-backend.h>

#include <elash/sema/expr/bin-op.h>
#include <elash/sema/expr/unary-op.h>
#include <elash/util/dynarena.h>
#include <elash/util/assert.h>
#include <elash/util/todo.h>

#include <llvm-c/Core.h>
#include <llvm-c/Target.h>
#include <llvm-c/Analysis.h>

#include <stdlib.h>

typedef ElcLLVMBackendFuncCtx FunctionContext;
typedef ElcLLVMBackendCtx     Context;

#define ASSIGN_REG(FUNC, MIR_VALUE, LLVM_VALUE, INSTR_NAME) \
    do { \
        EL_ASSERT((MIR_VALUE)->kind == EL_MIR_VAL_REG, INSTR_NAME " instr result should be a register"); \
        func->regs[(MIR_VALUE)->as.reg.id] = (LLVM_VALUE); \
    } while (0)

LLVMTypeRef elc_llvm_map_type(Context* ctx, ElType* type) {
    switch (type->kind) {
    case EL_TYPE_PRIM:
        switch (type->as.prim.kind) {
        case EL_PRIMTYPE_INT:
        case EL_PRIMTYPE_UINT:
            return LLVMInt32TypeInContext(ctx->context);
        case EL_PRIMTYPE_CHAR:
            return LLVMInt8TypeInContext(ctx->context);
        case EL_PRIMTYPE_BOOL:
            return LLVMInt1TypeInContext(ctx->context);
        case EL_PRIMTYPE_VOID:
            return LLVMVoidTypeInContext(ctx->context);
        }
        break;
    case EL_TYPE_FUNC: {
        LLVMTypeRef ret_type = elc_llvm_map_type(ctx, type->as.func.ret_type);
        LLVMTypeRef* param_types = malloc(sizeof(LLVMTypeRef) * type->as.func.param_count);
        for (usize i = 0; i < type->as.func.param_count; ++i) {
            param_types[i] = elc_llvm_map_type(ctx, type->as.func.params[i]);
        }
        LLVMTypeRef func_type = LLVMFunctionType(
            ret_type, param_types, type->as.func.param_count, /*IsVarArg=*/false
        );
        free(param_types);
        return func_type;
    }
    case EL_TYPE_ARRAY: {
        LLVMTypeRef base_type = elc_llvm_map_type(ctx, type->as.array.base);
        return LLVMArrayType(base_type, (unsigned)type->as.array.size);
    }
    case EL_TYPE_SLICE:
        EL_TODO("implement slice type");
    case EL_TYPE_RAW_SLICE:
    case EL_TYPE_PTR:
        return LLVMPointerTypeInContext(ctx->context, 0);
    }

    EL_UNREACHABLE_ENUM_VAL(ElTypeKind, type->kind);
}

LLVMValueRef elc_llvm_map_value(Context* ctx, FunctionContext* func, ElMirValue* value) {
    switch (value->kind) {
    case EL_MIR_VAL_CONST: {
        LLVMTypeRef type = elc_llvm_map_type(ctx, value->type);
        switch (value->type->as.prim.kind) {
        case EL_PRIMTYPE_INT:
            return LLVMConstInt(type, value->as.constant.lit.as.int_, true);
        case EL_PRIMTYPE_UINT:
            return LLVMConstInt(type, value->as.constant.lit.as.uint_, false);
        case EL_PRIMTYPE_CHAR:
            return LLVMConstInt(type, value->as.constant.lit.as.char_, false);
        case EL_PRIMTYPE_BOOL:
            return LLVMConstInt(type, value->as.constant.lit.as.bool_, false);
        case EL_PRIMTYPE_VOID:
            EL_UNREACHABLE("void constant");
        }
        EL_UNREACHABLE_ENUM_VAL(ElPrimitiveTypeKind, value->type->as.prim.kind);
    }
    case EL_MIR_VAL_ARG:
        return LLVMGetParam(func->llvm_fn, value->as.arg.idx);
    case EL_MIR_VAL_REG:
        return func->regs[value->as.reg.id];
    case EL_MIR_VAL_GLOBAL: {
        char* name = el_dynarena_make_cstr(ctx->arena, value->as.global.sym->name);
        LLVMValueRef glob = LLVMGetNamedFunction(ctx->current_mod, name);
        if (glob == NULL) {
            if (value->as.global.sym->kind == EL_SYM_FUNC) {
                LLVMTypeRef type = elc_llvm_map_type(ctx, value->as.global.sym->as.func.type);
                glob = LLVMAddFunction(ctx->current_mod, name, type);
            } else {
                glob = LLVMGetNamedGlobal(ctx->current_mod, name);
                if (glob == NULL) {
                    LLVMTypeRef type = elc_llvm_map_type(ctx, value->as.global.sym->as.var.type);
                    glob = LLVMAddGlobal(ctx->current_mod, type, name);
                    LLVMSetInitializer(glob, LLVMConstNull(type));
                }
            }
        }
        return glob;
    }
    }
    return NULL;
}

LLVMIntPredicate elc_llvm_get_predicate_of(ElSemaBinOp op, bool is_signed) {
    switch (op) {
    case EL_SEMA_BIN_OP_EQ:  return LLVMIntEQ;
    case EL_SEMA_BIN_OP_NEQ: return LLVMIntNE;
    case EL_SEMA_BIN_OP_GT:  return is_signed ? LLVMIntSGT : LLVMIntUGT;
    case EL_SEMA_BIN_OP_GTE: return is_signed ? LLVMIntSGE : LLVMIntUGE;
    case EL_SEMA_BIN_OP_LT:  return is_signed ? LLVMIntSLT : LLVMIntULT;
    case EL_SEMA_BIN_OP_LTE: return is_signed ? LLVMIntSLE : LLVMIntULE;
    default:
        EL_UNREACHABLE("op should be a comparison operator");
    }
}

bool elc_llvm_is_type_signed(const ElType* type) {
    if (type->kind == EL_TYPE_PRIM) {
        return type->as.prim.kind == EL_PRIMTYPE_INT;
    }
    return false;
}

void elc_llvm_compile_bin_instr(Context* ctx, FunctionContext* func, ElMirInstr* instr) {
    ElMirBinInstr* bin = &instr->as.bin;
    LLVMValueRef lhs = elc_llvm_map_value(ctx, func, bin->lhs);
    LLVMValueRef rhs = elc_llvm_map_value(ctx, func, bin->rhs);

    bool is_signed = elc_llvm_is_type_signed(bin->lhs->type);

    LLVMValueRef res = NULL;
    switch (bin->op) {
    case EL_SEMA_BIN_OP_ADD: res = LLVMBuildAdd(ctx->builder, lhs, rhs, ""); break;
    case EL_SEMA_BIN_OP_SUB: res = LLVMBuildSub(ctx->builder, lhs, rhs, ""); break;
    case EL_SEMA_BIN_OP_MUL: res = LLVMBuildMul(ctx->builder, lhs, rhs, ""); break;

    case EL_SEMA_BIN_OP_DIV: res = (is_signed ? LLVMBuildSDiv : LLVMBuildUDiv)(ctx->builder, lhs, rhs, ""); break;
    case EL_SEMA_BIN_OP_MOD: res = (is_signed ? LLVMBuildSRem : LLVMBuildURem)(ctx->builder, lhs, rhs, ""); break;

    case EL_SEMA_BIN_OP_AND: res = LLVMBuildAnd(ctx->builder, lhs, rhs, ""); break;
    case EL_SEMA_BIN_OP_OR:  res = LLVMBuildOr(ctx->builder, lhs, rhs, "");  break;

    case EL_SEMA_BIN_OP_BW_AND: res = LLVMBuildAnd(ctx->builder, lhs, rhs, ""); break;
    case EL_SEMA_BIN_OP_BW_OR:  res = LLVMBuildOr(ctx->builder, lhs, rhs, "");  break;
    case EL_SEMA_BIN_OP_BW_XOR: res = LLVMBuildXor(ctx->builder, lhs, rhs, ""); break;
    case EL_SEMA_BIN_OP_SHL:    res = LLVMBuildShl(ctx->builder, lhs, rhs, ""); break;
    case EL_SEMA_BIN_OP_SHR:
        res = (is_signed ? LLVMBuildAShr : LLVMBuildLShr)(ctx->builder, lhs, rhs, ""); break;

    default:
        if (el_sema_bin_op_is_comparison(bin->op)) {
            LLVMIntPredicate pred = elc_llvm_get_predicate_of(bin->op, is_signed);
            res = LLVMBuildICmp(ctx->builder, pred, lhs, rhs, "");
            break;
        }
        EL_UNREACHABLE_ENUM_VAL(ElSemaBinOp, bin->op);
    }

    ASSIGN_REG(func, instr->result, res, "binary");
}

void elc_llvm_compile_unary_instr(Context* ctx, FunctionContext* func, ElMirInstr* instr) {
    ElMirUnaryInstr* unary = &instr->as.unary;

    LLVMValueRef operand = elc_llvm_map_value(ctx, func, unary->operand);
    LLVMValueRef zero = LLVMConstInt(LLVMTypeOf(operand), 0, false);

    LLVMValueRef res = NULL;
    switch (unary->op) {
    case EL_SEMA_UNARY_OP_POS:
        res = operand;
        break;
    case EL_SEMA_UNARY_OP_NEG:
        res = LLVMBuildNeg(ctx->builder, operand, "");
        break;
    case EL_SEMA_UNARY_OP_NOT:
        res = LLVMBuildICmp(ctx->builder, LLVMIntEQ, operand, zero, "");
        break;
    case EL_SEMA_UNARY_OP_BW_NOT:
        res = LLVMBuildNot(ctx->builder, operand, "");
        break;

    case EL_SEMA_UNARY_OP_PRE_INC:
    case EL_SEMA_UNARY_OP_PRE_DEC:
    case EL_SEMA_UNARY_OP_POST_INC:
    case EL_SEMA_UNARY_OP_POST_DEC:
    case EL_SEMA_UNARY_OP_DEREF:
    case EL_SEMA_UNARY_OP_ADDROF:
        EL_UNREACHABLE("should be lowered before codegen");
        break;

    default:
        EL_UNREACHABLE_ENUM_VAL(ElSemaUnaryOp, unary->op);
    }

    ASSIGN_REG(func, instr->result, res, "unary");
}

void elc_llvm_compile_call_instr(Context* ctx, FunctionContext* func, ElMirInstr* instr) {
    ElMirCallInstr* call = &instr->as.call;

    LLVMValueRef callee    = elc_llvm_map_value(ctx, func, call->callee);
    LLVMTypeRef  func_type = elc_llvm_map_type(ctx, call->callee->type);

    LLVMValueRef* args = calloc(call->arg_count, sizeof(LLVMValueRef));
    for (uint32_t i = 0; i < call->arg_count; ++i) {
        args[i] = elc_llvm_map_value(ctx, func, call->args[i]);
    }

    LLVMValueRef result = LLVMBuildCall2(
        ctx->builder, 
        func_type, callee,
        args,
        call->arg_count,
        ""
    );

    free(args);
    if (instr->result != NULL) {
        ASSIGN_REG(func, instr->result, result, "call");
    }
}

void elc_llvm_compile_gep_instr(Context* ctx, FunctionContext* func, ElMirInstr* instr) {
    ElMirGepInstr* gep = &instr->as.gep;
    LLVMValueRef ptr = elc_llvm_map_value(ctx, func, gep->ptr);
    LLVMValueRef index = elc_llvm_map_value(ctx, func, gep->index);

    ElType* ptr_type = gep->ptr->type;
    EL_ASSERT(ptr_type->kind == EL_TYPE_PTR, "GEP source must be a pointer");

    ElType* base_type = ptr_type->as.ptr.base;
    LLVMTypeRef llvm_base_type = elc_llvm_map_type(ctx, base_type);

    LLVMValueRef res;
    if (base_type->kind == EL_TYPE_ARRAY) {
        LLVMValueRef indices[2] = {
            LLVMConstInt(LLVMInt32TypeInContext(ctx->context), 0, false),
            index
        };
        res = LLVMBuildGEP2(ctx->builder, llvm_base_type, ptr, indices, 2, "");
    } else {
        LLVMValueRef indices[1] = { index };
        res = LLVMBuildGEP2(ctx->builder, llvm_base_type, ptr, indices, 1, "");
    }

    ASSIGN_REG(func, instr->result, res, "gep");
}

void elc_llvm_compile_instr(Context* ctx, FunctionContext* func, ElMirInstr* instr) {
    switch (instr->kind) {
    case EL_MIR_INSTR_RET: {
        LLVMValueRef val = NULL;
        if (instr->as.return_.value != NULL) {
            val = elc_llvm_map_value(ctx, func, instr->as.return_.value);
        }
        LLVMBuildRet(ctx->builder, val);
        return;
    }

    case EL_MIR_INSTR_JMP:
        LLVMBuildBr(ctx->builder, func->blocks[instr->as.jmp.target_id]);
        return;
    case EL_MIR_INSTR_JMPIF: {
        LLVMValueRef cond = elc_llvm_map_value(ctx, func, instr->as.jmpif.cond);
        LLVMBuildCondBr(ctx->builder, cond, func->blocks[instr->as.jmpif.then_id], func->blocks[instr->as.jmpif.else_id]);
        return;
    }
    case EL_MIR_INSTR_UNREACHABLE:
        LLVMBuildUnreachable(ctx->builder);
        return;

    case EL_MIR_INSTR_ALLOCA: {
        LLVMTypeRef type = elc_llvm_map_type(ctx, instr->as.alloca.type);
        LLVMValueRef res = LLVMBuildAlloca(ctx->builder, type, "");
        ASSIGN_REG(func, instr->result, res, "alloca");
        return;
    }
    case EL_MIR_INSTR_LOAD: {
        LLVMTypeRef type = elc_llvm_map_type(ctx, instr->result->type);
        LLVMValueRef ptr = elc_llvm_map_value(ctx, func, instr->as.load.ptr);
        LLVMValueRef res = LLVMBuildLoad2(ctx->builder, type, ptr, "");
        ASSIGN_REG(func, instr->result, res, "load");
        return;
    }
    case EL_MIR_INSTR_STORE: {
        LLVMValueRef ptr = elc_llvm_map_value(ctx, func, instr->as.store.ptr);
        LLVMValueRef val = elc_llvm_map_value(ctx, func, instr->as.store.value);
        LLVMBuildStore(ctx->builder, val, ptr);
        return;
    }
    case EL_MIR_INSTR_GEP:
        elc_llvm_compile_gep_instr(ctx, func, instr);
        return;
    case EL_MIR_INSTR_BIN:
        elc_llvm_compile_bin_instr(ctx, func, instr);
        return;
    case EL_MIR_INSTR_UNARY:
        elc_llvm_compile_unary_instr(ctx, func, instr);
        return;
    case EL_MIR_INSTR_CALL:
        elc_llvm_compile_call_instr(ctx, func, instr);
        return;
    }

    EL_TODO("implement codegen for all instructions");
}

void elc_llvm_compile_func(Context* ctx, LLVMModuleRef module, ElMirFunc* mir_func) {
    LLVMTypeRef func_type = elc_llvm_map_type(ctx, mir_func->symbol->as.func.type);

    char* name = el_dynarena_make_cstr(ctx->arena, mir_func->symbol->name);

    FunctionContext func;
    func.llvm_fn = LLVMGetNamedFunction(module, name);
    if (!func.llvm_fn) {
        func.llvm_fn = LLVMAddFunction(module, name, func_type);
    }
    func.regs    = EL_DYNARENA_NEW_ARR_ZEROED(ctx->arena, LLVMValueRef, mir_func->reg_count);
    func.blocks  = EL_DYNARENA_NEW_ARR_ZEROED(ctx->arena, LLVMBasicBlockRef, mir_func->block_count);

    for (ElMirBlock* mir_block = mir_func->first_block; mir_block != NULL; mir_block = mir_block->next) {
        func.blocks[mir_block->id]
            = LLVMAppendBasicBlockInContext(ctx->context, func.llvm_fn, "block");
    }

    for (ElMirBlock* mir_block = mir_func->first_block; mir_block != NULL; mir_block = mir_block->next) {
        LLVMPositionBuilderAtEnd(ctx->builder, func.blocks[mir_block->id]);
        for (usize i = 0; i < mir_block->instr_count; ++i) {
            ElMirInstr* instr = mir_block->instructions[i];
            elc_llvm_compile_instr(ctx, &func, instr);
        }
    }
}

ElcCodegenResult elc_llvm_compile(
    ElcCodegenBackend* self,
    const ElMirModule* input,
    ElcLirHandle* output
) {
    Context* ctx = self->ctx;
    ctx->current_mod = LLVMModuleCreateWithNameInContext("elash-module", ctx->context);

    ElcLLVMLir* lir_data = EL_DYNARENA_NEW(ctx->arena, ElcLLVMLir);
    lir_data->module = ctx->current_mod;
    for (ElMirFunc* func = input->first_func; func != NULL; func = func->next) {
        elc_llvm_compile_func(ctx, lir_data->module, func);
    }

    LLVMVerifyModule(lir_data->module, LLVMPrintMessageAction, NULL);

    *output = elc_llvm_make_lir_handle(lir_data);
    return ELC_CODEGEN_OK;
}
