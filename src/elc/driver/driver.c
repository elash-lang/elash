#include <elc/driver/driver.h>
#include <elash/hir/dump/module.h>

#include <elc/driver/stages/lexer-stage.h>
#include <elc/driver/stages/pp-stage.h>
#include <elc/driver/stages/parser-stage.h>
#include <elc/driver/stages/binder-stage.h>
#include <elc/driver/stages/lowerer-stage.h>
#include <elc/driver/stages/codegen-stage.h>
#include <elc/driver/stages/emit-obj-stage.h>

#include <elc/driver/observers/dump-ast.h>
#include <elc/driver/observers/dump-hir.h>
#include <elc/driver/observers/dump-mir.h>
#include <elc/driver/observers/dump-lir.h>

bool elc_driver_init(ElcDriver* driver) {
    if (!el_dynarena_init(&driver->arena)) return false;
    el_diag_engine_init(&driver->diag, &driver->arena);
    el_sema_init_builtins(&driver->builtins, &driver->arena);
    elc_pipeline_init(&driver->pipeline, &driver->arena, &driver->diag, &driver->builtins);
    return true;
}

void elc_driver_free(ElcDriver* driver) {
    el_diag_engine_free(&driver->diag);
    el_dynarena_free(&driver->arena);
}

bool elc_driver_register_stages(ElcDriver* driver) {
    elc_pipeline_add_stage(&driver->pipeline, elc_make_lexer_stage());
    elc_pipeline_add_stage(&driver->pipeline, elc_make_pp_stage());
    elc_pipeline_add_stage(&driver->pipeline, elc_make_parser_stage());
    elc_pipeline_add_stage(&driver->pipeline, elc_make_binder_stage());
    elc_pipeline_add_stage(&driver->pipeline, elc_make_lowerer_stage());
    elc_pipeline_add_stage(&driver->pipeline, elc_make_codegen_stage());
    elc_pipeline_add_stage(&driver->pipeline, elc_make_emit_obj_stage());
    return true;
}

bool elc_driver_register_observers(ElcDriver* driver) {
    elc_pipeline_add_observer(&driver->pipeline, elc_make_dump_ast_observer());
    elc_pipeline_add_observer(&driver->pipeline, elc_make_dump_hir_observer());
    elc_pipeline_add_observer(&driver->pipeline, elc_make_dump_mir_observer());
    elc_pipeline_add_observer(&driver->pipeline, elc_make_dump_lir_observer());
    return true;
}

void elc_driver_provide_source(ElcDriver* driver, ElSourceDocument* source) {
    elc_pipeline_provide(&driver->pipeline, (ElcArtifact) {
        .kind = ELC_ART_SOURCE_TEXT,
        .as.source = source
    });
}

bool elc_driver_run(ElcDriver* driver, const char* output_path) {
    ElcArtifact out;
    if (!elc_pipeline_request(&driver->pipeline, ELC_ART_OBJ, &out)) {
        return false;
    }

    FILE* obj_file = fopen(output_path, "wb");
    if (!obj_file) {
        return false;
    }
    fwrite(out.as.obj.data, out.as.obj.size, 1, obj_file);
    fclose(obj_file);

    return true;
}
