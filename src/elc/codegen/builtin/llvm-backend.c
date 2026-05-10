#include <elc/codegen/builtin/llvm-backend.h>

#include <elash/util/dynarena.h>
#include <elash/util/assert.h>
#include <elash/util/todo.h>

#include <llvm-c/Core.h>
#include <llvm-c/Target.h>
#include <llvm-c/TargetMachine.h>

#include <stdlib.h>
#include <string.h>

void elc_llvm_lir_free(ElcLirHandle* handle) {
    ElcLLVMLir* data = handle->data;
    if (data->module) {
        LLVMDisposeModule(data->module);
    }
}

void elc_llvm_lir_dump(const ElcLirHandle* handle, FILE* out) {
    ElcLLVMLir* data = handle->data;
    char* ir = LLVMPrintModuleToString(data->module);
    fputs(ir, out);
    LLVMDisposeMessage(ir);
}

LLVMTargetMachineRef elc_llvm_create_target_machine(char** out_triple) {
    char* triple = LLVMGetDefaultTargetTriple();
    LLVMTargetRef target;
    char* error = NULL;
    if (LLVMGetTargetFromTriple(triple, &target, &error)) {
        LLVMDisposeMessage(error);
        LLVMDisposeMessage(triple);
        return NULL;
    }

    LLVMTargetMachineRef machine = LLVMCreateTargetMachine(
        target, triple, "generic", "", 
        LLVMCodeGenLevelDefault, LLVMRelocDefault, LLVMCodeModelDefault
    );

    if (out_triple) {
        *out_triple = triple;
    } else {
        LLVMDisposeMessage(triple);
    }

    return machine;
}

void elc_llvm_setup_module_layout(LLVMModuleRef module, LLVMTargetMachineRef machine, const char* triple) {
    LLVMSetTarget(module, triple);
    LLVMTargetDataRef target_data = LLVMCreateTargetDataLayout(machine);
    char* layout_str = LLVMCopyStringRepOfTargetData(target_data);
    LLVMSetDataLayout(module, layout_str);
    LLVMDisposeMessage(layout_str);
    LLVMDisposeTargetData(target_data);
}

ElcCodegenBuffer elc_llvm_lir_emit_to_buffer(const ElcLirHandle* handle, LLVMCodeGenFileType file_type) {
    ElcLLVMLir* data = handle->data;
    char* triple = NULL;
    LLVMTargetMachineRef machine = elc_llvm_create_target_machine(&triple);
    if (!machine) EL_TODO("Handle error");

    elc_llvm_setup_module_layout(data->module, machine, triple);

    LLVMMemoryBufferRef buffer_ref;
    char* error = NULL;
    if (LLVMTargetMachineEmitToMemoryBuffer(machine, data->module, file_type, &error, &buffer_ref)) {
        EL_TODO("Handle error");
    }

    ElcCodegenBuffer buffer = {
        .size = LLVMGetBufferSize(buffer_ref),
        .data = malloc(LLVMGetBufferSize(buffer_ref))
    };

    if (buffer.data) {
        memcpy(buffer.data, LLVMGetBufferStart(buffer_ref), buffer.size);
    } else {
        buffer.size = 0;
    }

    LLVMDisposeMemoryBuffer(buffer_ref);
    LLVMDisposeTargetMachine(machine);
    LLVMDisposeMessage(triple);

    return buffer;
}

ElcCodegenBuffer elc_llvm_lir_emit_obj(const ElcLirHandle* handle) {
    return elc_llvm_lir_emit_to_buffer(handle, LLVMObjectFile);
}
ElcCodegenBuffer elc_llvm_lir_emit_asm(const ElcLirHandle* handle) {
    return elc_llvm_lir_emit_to_buffer(handle, LLVMAssemblyFile);
}

void elc_llvm_lir_free_buffer(const ElcLirHandle* handle, ElcCodegenBuffer buffer) {
    (void) handle;
    free(buffer.data);
}

void elc_llvm_cleanup(ElcCodegenBackend* self) {
    ElcLLVMBackendCtx* ctx = self->ctx;
    LLVMDisposeBuilder(ctx->builder);
    LLVMContextDispose(ctx->context);
}

ElcLirHandle elc_llvm_make_lir_handle(ElcLLVMLir* data) {
    return (ElcLirHandle) {
        .ir_name     = EL_SV("llvm-ir"),
        .data        = data,
        .dump        = elc_llvm_lir_dump,
        .emit_asm    = elc_llvm_lir_emit_asm,
        .emit_obj    = elc_llvm_lir_emit_obj,
        .free        = elc_llvm_lir_free,
        .free_buffer = elc_llvm_lir_free_buffer,
    };
}

ElcCodegenBackend elc_make_llvm_codegen(ElDynArena* arena) {
    ElcLLVMBackendCtx* ctx = EL_DYNARENA_NEW(arena, ElcLLVMBackendCtx);
    ctx->context = LLVMContextCreate();
    ctx->builder = LLVMCreateBuilderInContext(ctx->context);
    ctx->arena = arena;
    
    LLVMInitializeNativeTarget();
    LLVMInitializeNativeAsmPrinter();

    return (ElcCodegenBackend) {
        .name = EL_SV("llvm"),
        .version = EL_SEM_VER(0, 1, 0),
        .ctx = ctx,
        .compile = elc_llvm_compile,
        .cleanup = elc_llvm_cleanup,
    };
}
