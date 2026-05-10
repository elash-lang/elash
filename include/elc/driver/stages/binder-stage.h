#pragma once
#include <elc/pipeline/pipeline.h>

bool elc_binder_stage_exec(const ElcStage* stage, ElcPipelineContext* ctx, const ElcArtifact* input, ElcArtifact* output);
ElcStage elc_make_binder_stage();

