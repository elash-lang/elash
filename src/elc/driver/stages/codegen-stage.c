#include <elc/driver/stages/codegen-stage.h>
#include <elc/codegen/llvm/codegen.h>

#include <elash/util/todo.h>

bool elc_codegen_stage_exec(const ElcStage* stage, ElcPipelineContext* ctx, const ElcArtifact* input, ElcArtifact* output) {
    (void) stage, (void) ctx;

    // TODO: support multiple backends
    ElcCodegenBackend backend = elc_make_llvm_codegen();

    ElcLirHandle handle = {0};

    ElcCodegenResult result = backend.compile(&backend, input->as.mir, &handle);
    if (result != ELC_CODEGEN_OK) {
        EL_TODO("error handling");
    }

    output->as.lir = handle;
    return true;
}

ElcStage elc_make_codegen_stage() {
    return (ElcStage) {
        .name = EL_SV("Codegen"),
        .execute = elc_codegen_stage_exec,

        .input_kind = ELC_ART_MIR,
        .output_kind = ELC_ART_LIR,
    };
}
