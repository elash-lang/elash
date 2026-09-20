#pragma once

#include <elash/util/dynarena.h>
#include <elash/diag/engine.h>

#include <elash/lexer/tokbuf.h>

#include <elash/lowerer/builtin.h>
#include <elash/binder/builtin.h>

#include <elash/sema/backends.h>
#include <elash/prof/prof.h>

#include <elash/pp/include.h>
#include <elash/source/doc.h>

#include <elc/pipeline/artifact.h>
#include <elc/codegen/backend.h>

#include <elash/sema/tcache.h>

typedef struct ElcPipelineContext {
    ElDynArena*   arena;
    ElProfState*  prof;
    ElDiagEngine* diag;
    bool debug;

    const ElPpIncMap* imap;
    const ElSourceDocument* root_src;

    ElBinderBuiltins*  binder_builtins;
    ElLowererBuiltins* lowerer_builtins;

    ElcCodegenBackend* backend;
    ElBSQuery*         bsquery;
    ElTypeCache        tcache;

    ElcOptLevel optlevel;

    ElTokenBuf* token_dump_bufs[ELC_ART_MAX];
} ElcPipelineContext;
