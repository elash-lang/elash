#include <elc/driver/stages/binder-stage.h>

#include <elash/binder/binder.h>

bool elc_binder_stage_exec(const ElcStage* stage, ElcPipelineContext* ctx, const ElcArtifact* input, ElcArtifact* output) {
    (void) stage;

    ElBinder binder;
    el_binder_init(&binder,
        .builtins = ctx->binder_builtins,
        .arena = ctx->arena,
        .diag = ctx->diag,

        .tcache  = &ctx->tcache,
        .bsquery = ctx->bsquery,
        .prof   = ctx->prof,
    );

    ElHirModule* mod = el_bind_module(&binder, input->as.ast);

    el_binder_free(&binder);
    if (el_diag_engine_has_errors(ctx->diag))
        return false;

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
