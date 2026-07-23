#include <elc/driver/observers/dump-ast.h>
#include <elash/ast/dump/module.h>
#include <stdio.h>
#include <string.h>

void elc_dump_ast_observer_exec(
    void* user_data,
    const ElcPipelineContext* ctx,
    ElcObserverEvent event,
    ElStringView stage_name,
    const ElcArtifact* artifact
) {
    (void) ctx, (void) stage_name;
    const char* path = (const char*) user_data;

    if (artifact == NULL || artifact->kind != ELC_ART_AST) return;

    switch (event) {
    case ELC_OBS_ARTIFACT_PRODUCED: {
        FILE* out = stdout;
        if (path && strcmp(path, "-") != 0) {
            out = fopen(path, "w");
            if (out == NULL) return;
        }

        fprintf(out, "ast dump:\n");
        el_ast_dump_module(artifact->as.ast, 0, out);
        fprintf(out, "\n");

        if (out != stdout) fclose(out);
        break;
    }
    default:
        break;
    }
}

ElcObserver elc_make_dump_ast_observer(const char* output_path) {
    return (ElcObserver) {
        .callback = elc_dump_ast_observer_exec,
        .user_data = (void*) output_path
    };
}
