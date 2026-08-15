#include <elc/driver/observers/dump-hir.h>
#include <elash/hir/dump/module.h>
#include <stdio.h>
#include <string.h>

void elc_dump_hir_observer_exec(
    void* user_data,
    const ElcPipelineContext* ctx,
    ElcObserverEvent event,
    ElStringView stage_name,
    const ElcArtifact* artifact
) {
    (void) ctx, (void) stage_name;
    const char* path = (const char*) user_data;

    if (artifact == NULL || artifact->kind != ELC_ART_HIR) return;
    if (event != ELC_OBS_END) return;

    FILE* out = stdout;
    if (path != NULL && strcmp(path, "-") != 0) {
        out = fopen(path, "w");
        if (out == NULL) return;
    }

    el_hir_dump_module(artifact->as.hir, 0, out);

    if (out != stdout) fclose(out);
}

ElcObserver elc_make_dump_hir_observer(const char* output_path) {
    return (ElcObserver) {
        .callback = elc_dump_hir_observer_exec,
        .user_data = (void*) output_path
    };
}
