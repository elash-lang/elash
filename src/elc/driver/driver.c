#include <elc/driver/driver.h>
#include <elash/util/todo.h>

#include <elash/defs/platform.h>
#include <elash/sema/tcache.h>
#include <elash/util/fopen.h>

#include <elash/diag/engine.h>
#include <elash/diag/handle.h>

#include <elash/diag/printer/console.h>
#include <elash/diag/printer/jsonl.h>

#include <elc/driver/stages/lexer-stage.h>
#include <elc/driver/stages/pp-stage.h>
#include <elc/driver/stages/parser-stage.h>
#include <elc/driver/stages/binder-stage.h>
#include <elc/driver/stages/lowerer-stage.h>
#include <elc/driver/stages/codegen-stage.h>
#include <elc/driver/stages/optimize-stage.h>
#include <elc/driver/stages/emit-obj-stage.h>
#include <elc/driver/stages/emit-asm-stage.h>

#include <elc/driver/observers/dump-tks.h>
#include <elc/driver/observers/dump-ast.h>
#include <elc/driver/observers/dump-hir.h>
#include <elc/driver/observers/dump-mir.h>
#include <elc/driver/observers/dump-lir.h>
#include <elc/driver/observers/timer.h>

#include <elc/codegen/builtin/llvm-backend.h>
#include <elash/prof/prof.h>

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
    elc_pipeline_cleanup(&driver->pipeline);
    el_diag_engine_free(&driver->diag);
    el_dynarena_free(&driver->arena);
}

bool elc_driver_register_stages(ElcDriver* driver) {
    elc_pipeline_add_stage(&driver->pipeline, elc_make_lexer_stage());
    elc_pipeline_add_stage(&driver->pipeline, elc_make_preproc_stage());
    elc_pipeline_add_stage(&driver->pipeline, elc_make_parser_stage());
    elc_pipeline_add_stage(&driver->pipeline, elc_make_binder_stage());
    elc_pipeline_add_stage(&driver->pipeline, elc_make_lowerer_stage());
    elc_pipeline_add_stage(&driver->pipeline, elc_make_codegen_stage());
    elc_pipeline_add_stage(&driver->pipeline, elc_make_optimize_stage());
    elc_pipeline_add_stage(&driver->pipeline, elc_make_emit_obj_stage());
    elc_pipeline_add_stage(&driver->pipeline, elc_make_emit_asm_stage());
    return true;
}

#define REGISTER_LIR_OBSERVER(KIND, FIELD)                                                          \
    do {                                                                                            \
        bool is_target = (args->emit == (KIND) || args->until == (KIND));                           \
        if (args->FIELD.is_enabled || is_target) {                                                  \
            const char* path = NULL;                                                                \
            if (args->FIELD.is_enabled && !el_sv_is_null(args->FIELD.output)) {                     \
                path = el_dynarena_make_cstr(&driver->arena, args->FIELD.output);                   \
            } else if (is_target) {                                                                 \
                path = el_dynarena_make_cstr(&driver->arena, args->output);                         \
            }                                                                                       \
            elc_pipeline_add_observer(&driver->pipeline,                                            \
                elc_make_dump_lir_observer(EL_DYNARENA_NEW_STRUCT(&driver->arena, ElcDumpLirData, { \
                    .path = path, .kind = (KIND)                                                    \
                }))                                                                                 \
            );                                                                                      \
        }                                                                                           \
    } while (0)

#define REGISTER_GENERIC_OBSERVER(KIND, FIELD, MAKER)                             \
    do {                                                                          \
        bool is_target = (args->emit == (KIND) || args->until == (KIND));         \
        if (args->FIELD.is_enabled || is_target) {                                \
            const char* path = NULL;                                              \
            if (args->FIELD.is_enabled && !el_sv_is_null(args->FIELD.output)) {   \
                path = el_dynarena_make_cstr(&driver->arena, args->FIELD.output); \
            } else if (is_target) {                                               \
                path = el_dynarena_make_cstr(&driver->arena, args->output);       \
            }                                                                     \
            elc_pipeline_add_observer(&driver->pipeline, (MAKER)(path));          \
        }                                                                         \
    } while (0)

#define REGISTER_TKS_OBSERVER(KIND, FIELD)                                              \
    do {                                                                                \
        if (args->FIELD.is_enabled) {                                                   \
            ElTokenBuf* buf = EL_DYNARENA_NEW(&driver->arena, ElTokenBuf);              \
            el_tkbuf_init(buf);                                                         \
            driver->pipeline.context.token_dump_bufs[(KIND)] = buf;                     \
            elc_pipeline_add_observer(&driver->pipeline, elc_make_dump_tokens_observer( \
                EL_DYNARENA_NEW_STRUCT(&driver->arena, ElcDumpTksData, {                \
                    .path = el_dynarena_make_cstr(&driver->arena, args->FIELD.output),  \
                    .buffer = buf, .kind = (KIND),                                      \
                })                                                                      \
            ));                                                                         \
        }                                                                               \
    } while (0)

// clang-tidy is stupid and doesn't understand macros
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
bool elc_driver_register_observers(ElcDriver* driver, const ElcArgs* args) {
    REGISTER_GENERIC_OBSERVER(ELC_ART_AST,  dump_ast, elc_make_dump_ast_observer);
    REGISTER_GENERIC_OBSERVER(ELC_ART_HIR,  dump_hir, elc_make_dump_hir_observer);
    REGISTER_GENERIC_OBSERVER(ELC_ART_MIR,  dump_mir, elc_make_dump_mir_observer);

    REGISTER_TKS_OBSERVER(ELC_ART_TKS, dump_toks);
    REGISTER_TKS_OBSERVER(ELC_ART_PPTKS, dump_pp_toks);

    REGISTER_LIR_OBSERVER(ELC_ART_LIR,  dump_lir);
    REGISTER_LIR_OBSERVER(ELC_ART_OLIR, dump_lir);

    if (args->time_reports.is_enabled) {
        elc_pipeline_add_observer(&driver->pipeline, elc_make_timer_observer());
    }

    return true;
}

#define STDIN_READ_BUF_SIZE 4096

static bool init_source_document(ElcDriver* driver, const ElcArgs* args, ElSourceDocument* src) {
    ElSrcDocStatus err;

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
        .kind = ELC_ART_SRC,
        .as.src = src
    });
    return true;
}

static ElcArtifactKind determine_target(const ElcArgs* args) {
    if (args->emit  != ELC_ART_NONE) return args->emit;
    if (args->until != ELC_ART_NONE) return args->until;
    return ELC_ART_OBJ;
}

#define pctx pipeline.context

static void init_backend(ElcDriver* driver) {
    // TODO: multi backend compiler

    ElcCodegenBackend* backend = EL_DYNARENA_NEW(&driver->arena, ElcCodegenBackend);
    *backend = elc_make_llvm_codegen(&driver->arena, &driver->pctx.tcache);
    driver->pctx.backend = backend;

    ElBSQuery* query = EL_DYNARENA_NEW(&driver->arena, ElBSQuery);
    *query = backend->query(backend);
    driver->pctx.bsquery = query;

    el_tcache_init(&driver->pctx.tcache, &driver->arena, driver->pctx.bsquery);
}

bool elc_driver_run(ElcDriver* driver, const ElcArgs* args) {
    driver->pctx.optlevel = args->opt;
    driver->pctx.imap     = &args->imap;

    ElProfState prof;
    if (args->time_reports.is_enabled) {
        el_prof_init(&prof, &driver->arena);
        driver->pctx.prof = &prof;
    } else {
        driver->pctx.prof = NULL;
    }

    ElSourceDocument src;
    if (!init_source_document(driver, args, &src)) {
        return false;
    }

    init_backend(driver);

    ElcArtifactKind target = determine_target(args);

    ElcArtifact out;
    bool success = elc_pipeline_request(&driver->pipeline, target, &out);

    if (success && (target == ELC_ART_OBJ || target == ELC_ART_ASM)) {
        ElcCodegenBuffer buffer = (target == ELC_ART_OBJ) ? out.as.obj : out.as.asm;

        ElStringView out_path = args->output;
        if (el_sv_eql(out_path, EL_SV("-")) && target == ELC_ART_OBJ) {
            out_path = EL_SV("output.o");
        }

        const char* path = el_dynarena_make_cstr(&driver->arena, out_path);

        bool needs_closing;
        FILE* f = el_open_ofile(path, &needs_closing);

        fwrite(buffer.data, 1, buffer.size, f);
        if (needs_closing) fclose(f);
    }

    if (driver->pctx.prof != NULL) {
        const char* path = el_dynarena_make_cstr(&driver->arena, args->time_reports.output);
        bool needs_closing;
        FILE* out = el_open_ofile(path, &needs_closing);
        if (out != NULL) {
            if (args->treport_format == ELC_TREPORT_JSONL) {
                el_prof_print_jsonl(driver->pctx.prof, out);
            } else {
                el_prof_print_console(driver->pctx.prof, out);
            }
            if (needs_closing) fclose(out);
        }
    }

    ElDiagPrinter printer;
    switch (args->dformat) {
    case ELC_DIAG_CONSOLE: printer = el_diag_make_console_printer(args->debug); break;
    case ELC_DIAG_JSONL:   printer = el_diag_make_jsonl_printer();              break;
    }
    el_diag_engine_print(&driver->diag, &printer, stdout);

    el_srcdoc_free(&src);
    return success;
}
