#include <elc/driver/stages/emit-asm-stage.h>

bool elc_emit_asm_stage_exec(const ElcStage* stage, ElcPipelineContext* ctx, const ElcArtifact* input, ElcArtifact* output) {
    (void) stage, (void) ctx;

    const ElcLirHandle* lir = &input->as.lir;
    ElcCodegenBuffer buffer = lir->emit_asm(lir);

    output->as.asm = buffer;
    return true;
}

ElcStage elc_make_emit_asm_stage() {
    return (ElcStage) {
        .name = EL_SV("Emit Assembly"),
        .execute = elc_emit_asm_stage_exec,

        .input_kind = ELC_ART_LIR,
        .output_kind = ELC_ART_ASM,
    };
}

