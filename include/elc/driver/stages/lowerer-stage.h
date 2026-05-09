#include <elc/pipeline/pipeline.h>

bool elc_lowerer_stage_exec(const ElcStage* stage, ElcPipelineContext* ctx, const ElcArtifact* input, ElcArtifact* output);
ElcStage elc_make_lowerer_stage();

