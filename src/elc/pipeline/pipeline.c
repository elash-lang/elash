#include <elc/pipeline/pipeline.h>

#include <elash/defs/int-types.h>
#include <elash/util/assert.h>

void elc_pipeline_init(ElcPipeline* pipeline, ElDynArena* arena, ElDiagEngine* diag) {
    *pipeline = (ElcPipeline) {0};
    pipeline->context.arena = arena;
    pipeline->context.diag = diag;
}

void elc_pipeline_add_stage(ElcPipeline* pipeline, ElcStage stage) {
    EL_ASSERT(pipeline->stage_count < ELC_MAX_STAGES, "Too many stages");
    pipeline->stages[pipeline->stage_count++] = stage;
}

void elc_pipeline_add_observer(ElcPipeline* pipeline, ElcObserver observer) {
    EL_ASSERT(pipeline->observer_count < ELC_MAX_OBSERVERS, "Too many observers");
    pipeline->observers[pipeline->observer_count++] = observer;
}

void elc_pipeline_provide(ElcPipeline* pipeline, ElcArtifact artifact) {
    pipeline->registry[artifact.kind] = artifact;
}

static void elc_pipeline_notify(
    ElcPipeline* pipeline,
    ElcObserverEvent event,
    const ElcStage* stage,
    const ElcArtifact* artifact
) {
    for (usize i = 0; i < pipeline->observer_count; ++i) {
        pipeline->observers[i].callback(
            pipeline->observers[i].user_data,
            &pipeline->context,
            event,
            stage->name,
            artifact
        );
    }
}

bool elc_pipeline_request(ElcPipeline* pipeline, ElcArtifactKind kind, ElcArtifact* out) {
    if (kind == ELC_ART_NONE) return true;

    // check if we already have it
    if (pipeline->registry[kind].kind != ELC_ART_NONE) {
        if (out) *out = pipeline->registry[kind];
        return true;
    }

    for (usize i = 0; i < pipeline->stage_count; ++i) {
        const ElcStage* stage = &pipeline->stages[i];
        if (stage->output_kind == kind) {
            ElcArtifact input;

            if (stage->input_kind != ELC_ART_NONE) {
                if (!elc_pipeline_request(pipeline, stage->input_kind, &input)) {
                    return false;
                }
            }

            elc_pipeline_notify(pipeline, ELC_OBS_STAGE_BEGIN, stage, NULL);

            ElcArtifact output = { .kind = kind };
            if (!stage->execute(stage, &pipeline->context, &input, &output)) {
                elc_pipeline_notify(pipeline, ELC_OBS_STAGE_END, stage, NULL);
                return false;
            }

            // just in case
            if (el_diag_engine_has_errors(pipeline->context.diag)) {
                return false;
            }

            pipeline->registry[kind] = output;
            elc_pipeline_notify(pipeline, ELC_OBS_STAGE_END, stage, NULL);
            elc_pipeline_notify(pipeline, ELC_OBS_ARTIFACT_PRODUCED, stage, &output);

            if (out) {
                *out = output;
            }
            return true;
        }
    }

    return false;
}
