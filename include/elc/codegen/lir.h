#pragma once

#include <elash/defs/int-types.h>
#include <elash/defs/sv.h>

typedef struct ElcCodegenBuffer {
    uint8_t* data;
    usize    size;
} ElcCodegenBuffer;

typedef struct ElcLirHandle ElcLirHandle;
typedef void ElcLirFreeFn   (ElcLirHandle* lir);
typedef void ElcLirDumpFn   (const ElcLirHandle* lir, FILE* out);

typedef ElcCodegenBuffer ElcLirEmitAsmFn(const ElcLirHandle* lir);
typedef ElcCodegenBuffer ElcLirEmitObjFn(const ElcLirHandle* lir);

typedef void ElcLirFreeBufferFn(const ElcLirHandle* lir, ElcCodegenBuffer buffer);

typedef struct ElcLirHandle {
    ElStringView ir_name; // for example "llvm ir"
    void* data;           // opaque pointer

    ElcLirDumpFn* dump;         // NULL if dump is unsupported
    ElcLirEmitAsmFn* emit_asm;  // NULL if emitting assembly is unsupported
    ElcLirEmitObjFn* emit_obj;  // always non-null

    ElcLirFreeFn* free;               // NULL if no cleanup is needed
    ElcLirFreeBufferFn* free_buffer;  // NULL if buffers don't need manual freeing
} ElcLirHandle;
