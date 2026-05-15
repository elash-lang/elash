#include <elc/driver/stages/parser-stage.h>

#include <elash/diag/engine.h>
#include <elash/parser/parser.h>

bool elc_parser_stage_exec(const ElcStage* stage, ElcPipelineContext* ctx, const ElcArtifact* input, ElcArtifact* output) {
    (void) stage;

    ElParser parser;
    el_parser_init(&parser, *input->as.tokens, ctx->diag, ctx->arena);

    ElAstModuleNode* mod = el_parser_parse_module(&parser);

    if (el_diag_engine_has_errors(ctx->diag)) {
        return false;
    }

    output->as.ast = mod;
    return true;
}

ElcStage elc_make_parser_stage() {
    return (ElcStage) {
        .name = EL_SV("Parser"),
        .execute = elc_parser_stage_exec,

        .input_kind = ELC_ART_PP_TOKENS,
        .output_kind = ELC_ART_AST,
    };
}
