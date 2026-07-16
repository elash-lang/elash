#include <elc/driver/stages/binder-stage.h>

#include <elash/binder/binder.h>

bool elc_binder_stage_exec(const ElcStage* stage, ElcPipelineContext* ctx, const ElcArtifact* input, ElcArtifact* output) {
    (void) stage;

    ElBinder binder;
    el_binder_init(&binder,
        .diag = ctx->diag, .builtins = ctx->binder_builtins,
        .hir_arena = ctx->arena, .sym_arena = ctx->arena, .type_arena = ctx->arena,
    );

    ElHirModule* mod = el_binder_bind_module(&binder, input->as.ast);

    el_binder_free(&binder);
    if (el_diag_engine_has_errors(ctx->diag)) {
        return false;
    }

    output->as.hir = mod;
    return true;
}

ElcStage elc_make_binder_stage() {
    return (ElcStage) {
        .name = EL_SV("Binder"),
        .execute = elc_binder_stage_exec,

        .input_kind = ELC_ART_AST,
        .output_kind = ELC_ART_HIR,
    };
}

