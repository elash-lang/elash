#include <elc/codegen/builtin/llvm-backend.h>

#include <elash/util/dynarena.h>
#include <elash/util/assert.h>
#include <elash/util/todo.h>

#include <llvm-c/Core.h>
#include <llvm-c/Target.h>

#include <stdlib.h>

typedef ElcLLVMBackendFuncCtx FunctionContext;
typedef ElcLLVMBackendCtx     Context;

LLVMTypeRef elc_llvm_map_type(Context* ctx, ElType* type) {
    switch (type->kind) {
    case EL_TYPE_PRIM:
        switch (type->as.prim.kind) {
        case EL_PRIMTYPE_INT:
        case EL_PRIMTYPE_UINT:
            return LLVMInt32TypeInContext(ctx->context);
        case EL_PRIMTYPE_CHAR:
            return LLVMInt8TypeInContext(ctx->context);
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
    case EL_TYPE_PTR:
        return LLVMPointerTypeInContext(ctx->context, 0);
    }

    EL_TODO("Unreachable or unhandled type");
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
        }
        EL_UNREACHABLE_ENUM_VAL(ElPrimitiveTypeKind, value->type->as.prim.kind);
    }
    case EL_MIR_VAL_ARG:
        return LLVMGetParam(func->llvm_fn, value->as.arg.idx);
    case EL_MIR_VAL_REG:
        return func->regs[value->as.reg.id];
    case EL_MIR_VAL_GLOBAL:
        EL_TODO("Global values not implemented");
    }
    return NULL;
}

void elc_llvm_compile_bin_instr(Context* ctx, FunctionContext* func, ElMirInstr* instr) {
    ElMirBinInstr* bin = &instr->as.bin;
    LLVMValueRef lhs = elc_llvm_map_value(ctx, func, bin->lhs);
    LLVMValueRef rhs = elc_llvm_map_value(ctx, func, bin->rhs);

    LLVMValueRef res = NULL;
    switch (bin->op) {
    case EL_SEMA_BIN_OP_ADD: res = LLVMBuildAdd(ctx->builder, lhs, rhs, ""); break;
    case EL_SEMA_BIN_OP_SUB: res = LLVMBuildSub(ctx->builder, lhs, rhs, ""); break;
    case EL_SEMA_BIN_OP_MUL: res = LLVMBuildMul(ctx->builder, lhs, rhs, ""); break;
    case EL_SEMA_BIN_OP_DIV: res = LLVMBuildSDiv(ctx->builder, lhs, rhs, ""); break; // TODO: handle signed/unsigned division
    default:
        EL_TODO("binary op not implemented");
    }

    EL_ASSERT(instr->result->kind == EL_MIR_VAL_REG, "binary instr result should be a register");
    func->regs[instr->result->as.reg.id] = res;
}

void elc_llvm_compile_instr(Context* ctx, FunctionContext* func, ElMirInstr* instr) {
    switch (instr->kind) {
    case EL_MIR_INSTR_RET: {
        LLVMValueRef val = NULL;
        if (instr->as.return_.value != NULL) {
            val = elc_llvm_map_value(ctx, func, instr->as.return_.value);
        }
        LLVMBuildRet(ctx->builder, val);
        break;
    }
    case EL_MIR_INSTR_BIN: {
        elc_llvm_compile_bin_instr(ctx, func, instr);
        break;
    }
    default:
        EL_TODO("implement codegen for all instructions");
    }
}

void elc_llvm_compile_func(Context* ctx, LLVMModuleRef module, ElMirFunc* mir_func) {
    LLVMTypeRef func_type = elc_llvm_map_type(ctx, mir_func->symbol->as.func.type);

    char* name = el_dynarena_make_cstr(ctx->arena, mir_func->symbol->name);

    FunctionContext func;
    func.llvm_fn = LLVMAddFunction(module, name, func_type);
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

    *output = elc_llvm_make_lir_handle(lir_data);
    return ELC_CODEGEN_OK;
}
