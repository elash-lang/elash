#pragma once

#include <elash/util/dynarena.h>
#include <elash/diag/engine.h>

#include <elash/binder/builtin.h>
#include <elash/lowerer/builtin.h>

#include <elc/pipeline/pipeline.h>
#include <elc/cli/args.h>

typedef struct ElcDriver {
    ElDynArena   arena;
    ElDiagEngine diag;
    ElcPipeline  pipeline;

    ElBinderBuiltins binder_builtins;
    ElLowererBuiltins lowerer_builtins;
} ElcDriver;

bool elc_driver_init(ElcDriver* driver);
void elc_driver_free(ElcDriver* driver);

bool elc_driver_register_stages(ElcDriver* driver);
bool elc_driver_register_observers(ElcDriver* driver, const ElcArgs* args);

bool elc_driver_run(ElcDriver* driver, const ElcArgs* args);
