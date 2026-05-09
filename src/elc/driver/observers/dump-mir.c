#include <elc/driver/observers/dump-mir.h>

#include <elash/mir/module.h>
#include <elash/mir/dump/module.h>

#include <stdio.h>

void elc_dump_mir_observer_exec(
    void* user_data,
    const ElcPipelineContext* ctx,
    ElcObserverEvent event,
    ElStringView stage_name,
    const ElcArtifact* artifact
) {
    (void) user_data, (void) ctx, (void) stage_name;

    if (artifact == NULL || artifact->kind != ELC_ART_MIR) return;

    switch (event) {
    case ELC_OBS_ARTIFACT_PRODUCED:
        puts("mir dump:");
        el_mir_dump_module(artifact->as.mir, stdout);
        break;
    default:
        break;
    }
}

ElcObserver elc_make_dump_mir_observer() {
    return (ElcObserver) {
        .callback = elc_dump_mir_observer_exec,
        .user_data = NULL
    };
}

