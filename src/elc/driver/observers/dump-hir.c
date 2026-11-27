#include <elc/driver/observers/dump-hir.h>
#include <elash/hir/dump/module.h>

#include <elash/util/fopen.h>
#include <elash/util/writer.h>
#include <stdio.h>

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

    bool needs_closing;
    FILE* out = el_open_ofile(path, &needs_closing);

    ElWriter writer = el_file_writer(out);
    el_hir_dump_module(artifact->as.hir, 0, &writer);

    if (needs_closing) fclose(out);
}

ElcObserver elc_make_dump_hir_observer(const char* output_path) {
    return (ElcObserver) {
        .callback = elc_dump_hir_observer_exec,
        .user_data = (void*) output_path
    };
}
