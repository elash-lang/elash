#include <elc/driver/stages/lowerer-stage.h>

#include <elash/lowerer/lowerer.h>

bool elc_lowerer_stage_exec(const ElcStage* stage, ElcPipelineContext* ctx, const ElcArtifact* input, ElcArtifact* output) {
    (void) stage;

    ElLowerer* lowerer = EL_DYNARENA_NEW(ctx->arena, ElLowerer);
    el_lowerer_init(lowerer, ctx->arena, ctx->diag);

    ElMirModule* mod = el_lowerer_lower_module(lowerer, input->as.hir);

    output->as.mir = mod;
    return true;
}

ElcStage elc_make_lowerer_stage() {
    return (ElcStage) {
        .name = EL_SV("Lowerer"),
        .execute = elc_lowerer_stage_exec,

        .input_kind = ELC_ART_HIR,
        .output_kind = ELC_ART_MIR,
    };
}


