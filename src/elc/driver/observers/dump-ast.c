#include <elc/driver/observers/dump-ast.h>
#include <elash/ast/dump/module.h>
#include <stdio.h>

void elc_dump_ast_observer_exec(
    void* user_data,
    const ElcPipelineContext* ctx,
    ElcObserverEvent event,
    ElStringView stage_name,
    const ElcArtifact* artifact
) {
    (void) user_data, (void) ctx, (void) stage_name;

    if (artifact == NULL || artifact->kind != ELC_ART_AST) return;

    switch (event) {
    case ELC_OBS_ARTIFACT_PRODUCED:
        puts("ast dump:");
        el_ast_dump_module(artifact->as.ast, 0, stdout);
        putchar('\n');
        break;
    default:
        break;
    }
}

ElcObserver elc_make_dump_ast_observer() {
    return (ElcObserver) {
        .callback = elc_dump_ast_observer_exec,
        .user_data = NULL
    };
}
