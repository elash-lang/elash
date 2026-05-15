#pragma once

#include <elash/util/dynarena.h>
#include <elash/sema/builtin.h>
#include <elash/diag/engine.h>

#include <elc/pipeline/pipeline.h>
#include <elc/cli/args.h>

typedef struct ElcDriver {
    ElDynArena   arena;
    ElDiagEngine diag;
    ElBuiltins   builtins;
    ElcPipeline  pipeline;
} ElcDriver;

bool elc_driver_init(ElcDriver* driver);
void elc_driver_free(ElcDriver* driver);

bool elc_driver_register_stages(ElcDriver* driver);
bool elc_driver_register_observers(ElcDriver* driver, const ElcArgs* args);

bool elc_driver_run(ElcDriver* driver, const ElcArgs* args);
