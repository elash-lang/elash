#pragma once

#include <elc/pipeline/stage.h>
#include <elc/pipeline/observer.h>

#define ELC_MAX_STAGES 32
#define ELC_MAX_OBSERVERS 16

typedef struct ElcPipeline {
    ElcPipelineContext context;
    
    ElcStage stages[ELC_MAX_STAGES];
    usize    stage_count;
    
    ElcObserver observers[ELC_MAX_OBSERVERS];
    usize       observer_count;
    
    ElcArtifact registry[ELC_ART_MAX];
} ElcPipeline;

void elc_pipeline_init(ElcPipeline* pipeline, ElDynArena* arena, ElDiagEngine* diag, ElBuiltins* builtins);
void elc_pipeline_add_stage(ElcPipeline* pipeline, ElcStage stage);
void elc_pipeline_add_observer(ElcPipeline* pipeline, ElcObserver observer);

void elc_pipeline_provide(ElcPipeline* pipeline, ElcArtifact artifact);
bool elc_pipeline_request(ElcPipeline* pipeline, ElcArtifactKind kind, ElcArtifact* out);
