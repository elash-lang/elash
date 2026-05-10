#pragma once
#include <elc/pipeline/pipeline.h>

bool elc_codegen_stage_exec(const ElcStage* stage, ElcPipelineContext* ctx, const ElcArtifact* input, ElcArtifact* output);
ElcStage elc_make_codegen_stage();

