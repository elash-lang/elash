#pragma once

#include <elash/util/dynarena.h>
#include <elash/diag/engine.h>

#include <elash/lowerer/builtin.h>
#include <elash/binder/builtin.h>

typedef struct ElcPipelineContext {
    ElDynArena*   arena;
    ElDiagEngine* diag;

    ElBinderBuiltins*  binder_builtins;
    ElLowererBuiltins* lowerer_builtins;
} ElcPipelineContext;
