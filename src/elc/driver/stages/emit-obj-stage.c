#include <elc/driver/stages/emit-obj-stage.h>

bool elc_emit_obj_stage_exec(const ElcStage* stage, ElcPipelineContext* ctx, const ElcArtifact* input, ElcArtifact* output) {
    (void) stage, (void) ctx;

    const ElcLirHandle* lir = &input->as.lir;
    ElcCodegenBuffer buffer = lir->emit_obj(lir);

    output->as.obj = buffer;
    return true;
}

ElcStage elc_make_emit_obj_stage() {
    return (ElcStage) {
        .name = EL_SV("Emit Object File"),
        .execute = elc_emit_obj_stage_exec,

        .input_kind = ELC_ART_LIR,
        .output_kind = ELC_ART_OBJ,
    };
}
