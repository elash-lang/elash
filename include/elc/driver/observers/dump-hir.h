#pragma once

#include <elc/pipeline/observer.h>

void elc_dump_hir_observer_exec(
    void* user_data,
    const ElcPipelineContext* ctx,
    ElcObserverEvent event,
    ElStringView stage_name,
    const ElcArtifact* artifact
);
ElcObserver elc_make_dump_hir_observer();

