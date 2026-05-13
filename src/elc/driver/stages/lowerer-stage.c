#include <elc/driver/stages/lowerer-stage.h>

#include <elash/lowerer/lowerer.h>

bool elc_lowerer_stage_exec(const ElcStage* stage, ElcPipelineContext* ctx, const ElcArtifact* input, ElcArtifact* output) {
    (void) stage;

    ElLowerer lowerer;
    el_lowerer_init(&lowerer, ctx->arena, ctx->diag, ctx->builtins);

    ElMirModule* mod = el_lowerer_lower_module(&lowerer, input->as.hir);

    el_lowerer_free(&lowerer);

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


