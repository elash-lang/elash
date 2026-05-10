#include <elc/codegen/llvm/codegen.h>

#include <llvm-c/Core.h>
#include <llvm-c/Target.h>
#include <llvm-c/TargetMachine.h>

#include <stdlib.h>

typedef struct {
    LLVMContextRef context;
} BackendContext;

typedef struct {
    LLVMModuleRef module;
} LirHandleData;

static void elc_llvm_lir_free(ElcLirHandle* handle) {
    LirHandleData* data = handle->data;
    if (data->module) {
        LLVMDisposeModule(data->module);
    }
    free(data);
}

static void elc_llvm_lir_dump(const ElcLirHandle* handle, FILE* out) {
    LirHandleData* data = handle->data;
    char* ir = LLVMPrintModuleToString(data->module);
    fputs(ir, out);
    LLVMDisposeMessage(ir);
}

static ElcCodegenBuffer elc_llvm_lir_emit_obj(const ElcLirHandle* handle) {
    (void) handle;
    ElcCodegenBuffer buffer;
    buffer.size = 0;
    buffer.data = NULL;
    return buffer;
}

static void elc_llvm_lir_free_buffer(const ElcLirHandle* handle, ElcCodegenBuffer buffer) {
    (void) handle;
    if (buffer.data) {
        free(buffer.data);
    }
}

ElcLirHandle elc_llvm_make_lir_handle(LirHandleData* data) {
    return (ElcLirHandle) {
        .ir_name = EL_SV("llvm-ir"),
        .data = data,
        .dump = elc_llvm_lir_dump,
        .emit_asm = NULL, // TODO: emitting assembly support
        .emit_obj = elc_llvm_lir_emit_obj,
        .free = elc_llvm_lir_free,
        .free_buffer = elc_llvm_lir_free_buffer,
    };
}

static ElcCodegenResult elc_llvm_compile(
    ElcCodegenBackend* self,
    const ElMirModule* input,
    ElcLirHandle* output
) {
    (void) input;
    BackendContext* ctx = (BackendContext*)self->ctx;
    
    LirHandleData* lir_data = malloc(sizeof(LirHandleData));
    lir_data->module = LLVMModuleCreateWithNameInContext("elash-module", ctx->context);
   
    *output = elc_llvm_make_lir_handle(lir_data);
    return ELC_CODEGEN_OK;
}

static void elc_llvm_cleanup(ElcCodegenBackend* self) {
    BackendContext* ctx = self->ctx;
    if (ctx->context) {
        LLVMContextDispose(ctx->context);
    }
    free(ctx);
}

ElcCodegenBackend elc_make_llvm_codegen() {
    BackendContext* ctx = malloc(sizeof(BackendContext));
    ctx->context = LLVMContextCreate();
    
    return (ElcCodegenBackend) {
        .name = EL_SV("llvm"),
        .version = EL_SEM_VER(0, 1, 0),
        .ctx = ctx,
        .compile = elc_llvm_compile,
        .cleanup = elc_llvm_cleanup,
    };
}
