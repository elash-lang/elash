#include <elc/driver/observers/dump-tks.h>
#include <elash/lexer/token.h>
#include <stdio.h>
#include <string.h>

void elc_dump_tokens_observer_exec(
    void* user_data,
    const ElcPipelineContext* ctx,
    ElcObserverEvent event,
    ElStringView stage_name,
    const ElcArtifact* artifact
) {
    (void) ctx, (void) stage_name;
    ElcDumpTksData* data = user_data;

    if (artifact == NULL || artifact->kind != data->kind) return;
    if (event != ELC_OBS_END) return;

    FILE* out = stdout;
    if (data->path != NULL && strcmp(data->path, "-") != 0) {
        out = fopen(data->path, "w");
        if (out == NULL) return;
    }

    if (data->buffer != NULL) {
        for (usize i = 0; i < data->buffer->len; ++i) {
            el_token_print(&data->buffer->data[i], out);
            fprintf(out, "\n");
        }
    }

    if (out != stdout) fclose(out);
}

ElcObserver elc_make_dump_tokens_observer(ElcDumpTksData* data) {
    return (ElcObserver) {
        .callback = elc_dump_tokens_observer_exec,
        .user_data = data
    };
}
