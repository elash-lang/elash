#include <elash/diag/engine.h>
#include <elash/diag/handle.h>
#include <elash/diag/printer/console.h>

#include <elc/driver/driver.h>

#include <elc/cli/args.h>
#include <elc/cli/parser.h>

int main(int argc, const char* const* argv) {
    ElcArgs args;
    ElcCliParseResult result = elc_cli_parse_args(argc, argv, &args);
    if (result.code != ELC_CLI_PARSE_OK) {
        elc_cli_print_error(stderr, result);
        return 1;
    }

    ElcDriver driver;
    if (!elc_driver_init(&driver)) {
        fprintf(stderr, "failed to initialize driver\n");
        return 1;
    }

    elc_driver_register_stages(&driver);
    elc_driver_register_observers(&driver, &args);

    bool success = elc_driver_run(&driver, &args);

    ElDiagPrinter printer = el_diag_make_console_printer();
    el_diag_engine_print(&driver.diag, &printer, stdout);

    elc_driver_free(&driver);

    return success ? 0 : 1;
}
