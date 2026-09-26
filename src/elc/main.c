#include <elc/driver/driver.h>
#include <elc/driver/crash.h>

#include <elash/util/ansi.h>

#include <elc/cli/args.h>
#include <elc/cli/argparse.h>

int main(int argc, const char* argv[]) {
    ElcDriver driver;
    if (!elc_driver_init(&driver)) {
        fprintf(stderr, "failed to initialize driver\n");
        return 1;
    }

    ElcArgs args;
    ElcCliParseResult result = elc_cli_parse_args(argc, argv, &args, &driver.arena);
    if (result.code != ELC_CLI_PARSE_OK) {
        elc_cli_print_error(stderr, result);
        return 1;
    }

    // currently auto is equivalent to always, this may change in the future
    if (args.crash_handlers != ELC_PREF_NEVER) {
        elc_register_crash_handlers();
    }

    el_ansi_init(
        args.color == ELC_PREF_ALWAYS
            ? EL_ANSI_ENABLED
            : args.color == ELC_PREF_AUTO
                ? EL_ANSI_AUTO
                : EL_ANSI_DISABLED);

    if (args.help) {
        // in elash this would be
        //  argv[?0] ?? "elc"
        // very advanced tech btw.
        if (argv[0] == NULL) argv[0] = "elc";
        elc_cli_print_usage(stdout, argv[0]);
        return 0;
    }
    if (args.version) {
        elc_cli_print_version(stdout);
        return 0;
    }

    elc_driver_register_stages(&driver);
    elc_driver_register_observers(&driver, &args);

    bool success = elc_driver_run(&driver, &args);
    elc_driver_free(&driver);

    return success ? 0 : 1;
}
