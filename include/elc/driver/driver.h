#pragma once

#include <elash/util/dynarena.h>
#include <elash/diag/engine.h>

#include <elc/pipeline/pipeline.h>

typedef struct ElcDriver {
    ElDynArena arena;
    ElDiagEngine diag;
    ElcPipeline pipeline;
} ElcDriver;

bool elc_driver_init(ElcDriver* driver);
void elc_driver_free(ElcDriver* driver);

bool elc_driver_register_stages(ElcDriver* driver);
bool elc_driver_register_observers(ElcDriver* driver);

void elc_driver_provide_source(ElcDriver* driver, ElSourceDocument* source);

bool elc_driver_run(ElcDriver* driver, const char* output_path);
