#pragma once

#include <elc/codegen/backend.h>
#include <elash/util/dynarena.h>

#include <llvm-c/Core.h>
#include <llvm-c/Target.h>

typedef struct ElcLLVMBackendCtx {
    LLVMContextRef context;
    LLVMBuilderRef builder;
    ElDynArena* arena;

    LLVMModuleRef current_mod;
    LLVMValueRef current_func;
} ElcLLVMBackendCtx;

typedef struct {
    LLVMModuleRef module;
} ElcLLVMLir;

ElcBackendCompileFn elc_llvm_compile;
ElcLirHandle elc_llvm_make_lir_handle(ElcLLVMLir* data);
ElcCodegenBackend elc_make_llvm_codegen(ElDynArena* arena);
