#pragma once

#include <elash/defs/sv.h>

typedef struct ElcLirHandle ElcLirHandle;
typedef void ElcLirFreeFn   (ElcLirHandle* lir);
typedef void ElcLirDumpFn   (const ElcLirHandle* lir, FILE* out);
typedef void ElcLirEmitAsmFn(const ElcLirHandle* lir, FILE* out);
typedef void ElcLirEmitObjFn(const ElcLirHandle* lir, FILE* out);

typedef struct ElcLirHandle {
    ElStringView ir_name; // for example "llvm ir"
    void* data;           // opaque pointer

    ElcLirDumpFn* dump;         // NULL if dump is unsupported
    ElcLirEmitAsmFn* emit_asm;  // NULL if emitting assembly is unsupported
    ElcLirEmitObjFn* emit_obj;  // always non-null

    ElcLirFreeFn* free; // NULL if no cleanup is needed
} ElcLirHandle;
