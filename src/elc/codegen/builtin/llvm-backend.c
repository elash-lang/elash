#include <elc/codegen/builtin/llvm-backend.h>

#include <elash/util/dynarena.h>
#include <elash/util/todo.h>

#include <llvm-c/Core.h>
#include <llvm-c/Target.h>
#include <llvm-c/TargetMachine.h>

#include <stdlib.h>

typedef struct {
    LLVMContextRef context;
    ElDynArena* arena;
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

LLVMTypeRef elc_llvm_map_type(BackendContext* ctx, ElType* type) {
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

static ElcCodegenResult elc_llvm_compile(
    ElcCodegenBackend* self,
    const ElMirModule* input,
    ElcLirHandle* output
) {
    BackendContext* ctx = self->ctx;

    LirHandleData* lir_data = malloc(sizeof(LirHandleData));
    lir_data->module = LLVMModuleCreateWithNameInContext("elash-module", ctx->context);

    for (ElMirFunc* func = input->first_func; func != NULL; func = func->next) {
        LLVMTypeRef func_type = elc_llvm_map_type(ctx, func->symbol->as.func.type);

        char* name = el_dynarena_make_cstr(ctx->arena, func->symbol->name);
        LLVMAddFunction(lir_data->module, name, func_type);
    }

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

ElcCodegenBackend elc_make_llvm_codegen(ElDynArena* arena) {
    BackendContext* ctx = malloc(sizeof(BackendContext));
    ctx->context = LLVMContextCreate();
    ctx->arena = arena;
    
    return (ElcCodegenBackend) {
        .name = EL_SV("llvm"),
        .version = EL_SEM_VER(0, 1, 0),
        .ctx = ctx,
        .compile = elc_llvm_compile,
        .cleanup = elc_llvm_cleanup,
    };
}
