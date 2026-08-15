#include <elc/pipeline/pipeline.h>

#include <elash/defs/int-types.h>
#include <elash/util/assert.h>

void elc_pipeline_init(
    ElcPipeline* pipeline, ElDynArena* arena, ElDiagEngine* diag,
    ElBinderBuiltins* binder_builtins,
    ElLowererBuiltins* lowerer_builtins,
    ElcOptLevel optlvl
) {
    *pipeline = (ElcPipeline) {
        .context.arena    = arena,
        .context.diag     = diag,
        .context.optlevel = optlvl,

        .context.binder_builtins = binder_builtins,
        .context.lowerer_builtins = lowerer_builtins,
    };
}

void elc_pipeline_cleanup(ElcPipeline* pipeline) {
    for (usize i = 0; i < ELC_ART_MAX; ++i) {
        if (pipeline->registry[i].kind != ELC_ART_NONE) {
            elc_artifact_free(&pipeline->registry[i]);
        }
    }

    if (pipeline->context.backend != NULL) {
        ElcBackendCleanupFn* cleanup = pipeline->context.backend->cleanup;
        if (cleanup != NULL) cleanup(pipeline->context.backend);
    }
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
        if (out != NULL) *out = pipeline->registry[kind];
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

            elc_pipeline_notify(pipeline, ELC_OBS_START, stage, NULL);

            ElcArtifact output = { .kind = kind };
            if (!stage->execute(stage, &pipeline->context, &input, &output)) {
                elc_pipeline_notify(pipeline, ELC_OBS_FINISH, stage, NULL);
                return false;
            }

            // just in case
            if (el_diag_engine_has_errors(pipeline->context.diag)) {
                return false;
            }

            pipeline->registry[kind] = output;
            elc_pipeline_notify(pipeline, ELC_OBS_FINISH, stage, &output);
            elc_pipeline_notify(pipeline, ELC_OBS_END, stage, &output);

            if (out != NULL) *out = output;
            return true;
        }
    }

    return false;
}
