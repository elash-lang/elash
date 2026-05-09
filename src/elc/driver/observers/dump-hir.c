#include <elc/driver/observers/dump-hir.h>

#include <elash/hir/tree/module.h>
#include <elash/hir/dump/module.h>

#include <stdio.h>

void elc_dump_hir_observer_exec(
    void* user_data,
    const ElcPipelineContext* ctx,
    ElcObserverEvent event,
    ElStringView stage_name,
    const ElcArtifact* artifact
) {
    (void) user_data, (void) ctx, (void) stage_name;

    if (artifact == NULL || artifact->kind != ELC_ART_HIR) return;

    switch (event) {
    case ELC_OBS_ARTIFACT_PRODUCED:
        puts("hir dump:");
        el_hir_dump_module(artifact->as.hir, 0, stdout);
        break;
    default:
        break;
    }
}

ElcObserver elc_make_dump_hir_observer() {
    return (ElcObserver) {
        .callback = elc_dump_hir_observer_exec,
        .user_data = NULL
    };
}

