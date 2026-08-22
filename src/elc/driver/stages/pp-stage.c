#include <elc/driver/stages/pp-stage.h>

#include <elash/pp/preproc.h>

bool elc_pp_stage_exec(const ElcStage* stage, ElcPipelineContext* ctx, const ElcArtifact* input, ElcArtifact* output) {
    (void) stage;

    ElPreproc* pp = EL_DYNARENA_NEW(ctx->arena, ElPreproc);
    if (!el_pp_init(pp, *input->as.tks, ctx->root_src, ctx->arena, ctx->imap)) {
        // TODO: error handling
        return false;
    }

    ElTokenStream stream = el_pp_as_token_stream(pp);

    if (ctx->token_dump_bufs[ELC_ART_PPTKS] != NULL) {
        el_tkbuf_capture_stream(&stream, ctx->token_dump_bufs[ELC_ART_PPTKS], ctx->diag);

        ElTkBufStream* tkbuf_ctx = EL_DYNARENA_NEW(ctx->arena, ElTkBufStream);
        stream = el_tkbuf_as_stream(tkbuf_ctx, ctx->token_dump_bufs[ELC_ART_PPTKS]);
    }

    ElTokenStream* stream_ptr = EL_DYNARENA_NEW(ctx->arena, ElTokenStream);
    *stream_ptr = stream;

    output->as.tks = stream_ptr;
    return true;
}

ElcStage elc_make_pp_stage() {
    return (ElcStage) {
        .name = EL_SV("Preprocessor"),
        .execute = elc_pp_stage_exec,

        .input_kind = ELC_ART_TKS,
        .output_kind = ELC_ART_PPTKS,
    };
}
