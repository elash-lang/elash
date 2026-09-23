#pragma once

#include <elc/codegen/backend.h>
#include <elash/util/dynarena.h>
#include <elash/sema/tcache.h>
#include <elash/prof/prof.h>

#include <llvm-c/Core.h>

#include <llvm-c/TargetMachine.h>
#include <llvm-c/Target.h>

typedef struct ElcLLVMBackendFuncCtx {
    LLVMValueRef       llvm_fn;
    LLVMValueRef*      regs;
    LLVMBasicBlockRef* blocks;
} ElcLLVMBackendFuncCtx;

typedef struct ElcLLVMTargetData {
    LLVMTargetMachineRef machine;
    LLVMTargetDataRef    data;

    char* triple;
} ElcLLVMTargetData;

typedef struct ElcLLVMBackendCtx {
    LLVMContextRef context;
    LLVMBuilderRef builder;

    ElTypeCache* tcache;
    ElDynArena*  arena;
    ElProfState* prof;

    ElProfSubstage
        *pss_types,
        *pss_instr,
        *pss_const;

    bool cached_query;
    ElBSQuery query;

    LLVMModuleRef current_mod;
    LLVMValueRef* globals;
    uint32_t      globals_count;

    ElcLLVMTargetData target;
} ElcLLVMBackendCtx;

typedef struct {
    LLVMModuleRef      module;
    ElcLLVMTargetData* target;
} ElcLLVMLir;

ElBSQuery _elc_llvm_query(ElcLLVMBackendCtx* ctx);

ElcBackendCompileFn elc_llvm_compile;
ElcBackendQueryFn   elc_llvm_query;

ElcLirHandle elc_llvm_make_lir_handle(ElcLLVMLir* data);
ElcCodegenBackend elc_make_llvm_codegen(ElDynArena* arena, ElTypeCache* tcache, ElProfState* prof);

LLVMTypeRef elc_llvm_map_type(ElcLLVMBackendCtx* ctx, const ElMirType* type);
void elc_llvm_setup_module_layout(LLVMModuleRef module, LLVMTargetDataRef target_data, const char* triple);
