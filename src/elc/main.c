#include <elash/diag/engine.h>
#include <elash/diag/handle.h>
#include <elash/diag/printer/console.h>

#include <elc/driver/driver.h>

int main(int argc, const char* const* argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: %s source-code.ela\n", argv[0]);
        return 1;
    }

    const char* input_path = argv[1];

    ElcDriver driver;
    if (!elc_driver_init(&driver)) {
        fprintf(stderr, "failed to initialize driver\n");
        return 1;
    }

    ElSourceDocument source;
    if (el_srcdoc_init_from_file(&source, input_path) != EL_SRCDOC_ERR_SUCCESS) {
        fprintf(stderr, "failed to load input file: %s\n", input_path);
        elc_driver_free(&driver);
        return 1;
    }

    elc_driver_provide_source(&driver, &source);
    elc_driver_register_stages(&driver);
    elc_driver_register_observers(&driver);

    bool success = elc_driver_run(&driver);

    ElDiagPrinter printer = el_diag_make_console_printer();
    el_diag_engine_print(&driver.diag, &printer, stdout);

    el_srcdoc_destroy(&source);
    elc_driver_free(&driver);

    return success ? 0 : 1;
}
