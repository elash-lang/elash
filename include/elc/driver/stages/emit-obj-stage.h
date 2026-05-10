#pragma once
#include <elc/pipeline/pipeline.h>

bool elc_emit_obj_stage_exec(const ElcStage* stage, ElcPipelineContext* ctx, const ElcArtifact* input, ElcArtifact* output);
ElcStage elc_make_emit_obj_stage();

