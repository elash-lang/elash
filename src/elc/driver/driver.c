#include <elc/driver/driver.h>

#include <elash/diag/engine.h>
#include <elash/diag/handle.h>
#include <elash/diag/printer/console.h>

#include <elc/driver/stages/lexer-stage.h>
#include <elc/driver/stages/pp-stage.h>
#include <elc/driver/stages/parser-stage.h>
#include <elc/driver/stages/binder-stage.h>
#include <elc/driver/stages/lowerer-stage.h>
#include <elc/driver/stages/codegen-stage.h>
#include <elc/driver/stages/emit-obj-stage.h>
#include <elc/driver/stages/emit-asm-stage.h>

#include <elc/driver/observers/dump-ast.h>
#include <elc/driver/observers/dump-hir.h>
#include <elc/driver/observers/dump-mir.h>
#include <elc/driver/observers/dump-lir.h>

bool elc_driver_init(ElcDriver* driver) {
    if (!el_dynarena_init(&driver->arena)) return false;
    el_diag_engine_init(&driver->diag, &driver->arena);
    el_binder_init_builtins(&driver->binder_builtins, &driver->arena);
    el_lowerer_init_builtins(&driver->lowerer_builtins, &driver->arena);

    elc_pipeline_init(
        &driver->pipeline, &driver->arena, &driver->diag,
        &driver->binder_builtins, &driver->lowerer_builtins
    );

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
    elc_pipeline_add_stage(&driver->pipeline, elc_make_emit_asm_stage());
    return true;
}

#define REGISTER_DUMP_OBSERVER(KIND, FIELD, MAKER) \
    do { \
        bool is_target = (args->emit == (KIND) || args->until == (KIND)); \
        if (args->FIELD.is_enabled || is_target) { \
            const char* path = NULL; \
            if (args->FIELD.is_enabled && !el_sv_is_null(args->FIELD.output)) { \
                path = el_dynarena_make_cstr(&driver->arena, args->FIELD.output); \
            } else if (is_target) { \
                path = el_dynarena_make_cstr(&driver->arena, args->output); \
            } \
            elc_pipeline_add_observer(&driver->pipeline, (MAKER)(path)); \
        } \
    } while (0)

// clang-tidy is stupid and dont understand macros
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
bool elc_driver_register_observers(ElcDriver* driver, const ElcArgs* args) {
    REGISTER_DUMP_OBSERVER(ELC_ART_AST, dump_ast, elc_make_dump_ast_observer);
    REGISTER_DUMP_OBSERVER(ELC_ART_HIR, dump_hir, elc_make_dump_hir_observer);
    REGISTER_DUMP_OBSERVER(ELC_ART_MIR, dump_mir, elc_make_dump_mir_observer);
    REGISTER_DUMP_OBSERVER(ELC_ART_LIR, dump_lir, elc_make_dump_lir_observer);
    return true;
}

#define STDIN_READ_BUF_SIZE 4096

static bool init_source_document(ElcDriver* driver, const ElcArgs* args, ElSourceDocument* src) {
    ElSrcDocErrorCode err;

    if (el_sv_eql(args->input, EL_SV("-"))) {
        err = el_srcdoc_init_empty(src, EL_SV("<stdin>"));
        if (err == EL_SRCDOC_ERR_SUCCESS) {
            char buf[STDIN_READ_BUF_SIZE];
            size_t n;
            while ((n = fread(buf, 1, sizeof(buf), stdin)) > 0) {
                el_srcdoc_append_str(src, el_sv_from_data_and_len(buf, n));
            }
        }
    } else {
        err = el_srcdoc_init_from_file(src, el_dynarena_make_cstr(&driver->arena, args->input));
    }

    if (err != EL_SRCDOC_ERR_SUCCESS) return false;

    elc_pipeline_provide(&driver->pipeline, (ElcArtifact) {
        .kind = ELC_ART_SOURCE_TEXT,
        .as.source = src
    });

    return true;
}

static ElcArtifactKind determine_target(const ElcArgs* args) {
    if (args->emit != ELC_ART_NONE)  return args->emit;
    if (args->until != ELC_ART_NONE) return args->until;
    return ELC_ART_OBJ;
}

bool elc_driver_run(ElcDriver* driver, const ElcArgs* args) {
    ElSourceDocument src;
    if (!init_source_document(driver, args, &src)) return false;

    ElcArtifactKind target = determine_target(args);

    ElcArtifact out;
    bool success = elc_pipeline_request(&driver->pipeline, target, &out);

    if (success && (target == ELC_ART_OBJ || target == ELC_ART_ASM)) {
        ElcCodegenBuffer buffer = (target == ELC_ART_OBJ) ? out.as.obj : out.as.asm;

        ElStringView out_path = args->output;
        if (el_sv_eql(out_path, EL_SV("-")) && target == ELC_ART_OBJ) {
            out_path = EL_SV("output.o");
        }

        FILE* f = NULL;
        if (el_sv_eql(out_path, EL_SV("-"))) {
            f = stdout;
        } else if (!el_sv_is_null(out_path)) {
            const char* path = el_dynarena_make_cstr(&driver->arena, out_path);
            f = fopen(path, "wb");
        }

        if (f != NULL) {
            fwrite(buffer.data, 1, buffer.size, f);
            if (f != stdout) fclose(f);
        } else {
            success = false;
        }
    }

    ElDiagPrinter printer = el_diag_make_console_printer();
    el_diag_engine_print(&driver->diag, &printer, stdout);

    el_srcdoc_destroy(&src);
    return success;
}
