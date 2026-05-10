#pragma once
#include <elc/pipeline/pipeline.h>

bool elc_parser_stage_exec(const ElcStage* stage, ElcPipelineContext* ctx, const ElcArtifact* input, ElcArtifact* output);
ElcStage elc_make_parser_stage();
