#include <elc/driver/driver.h>

#include <elc/cli/args.h>
#include <elc/cli/argparse.h>

int main(int argc, const char* argv[]) {
    ElcArgs args;
    ElcCliParseResult result = elc_cli_parse_args(argc, argv, &args);
    if (result.code != ELC_CLI_PARSE_OK) {
        elc_cli_print_error(stderr, result);
        return 1;
    }

    if (args.help) {
        // in elash this would be
        //  argv[?0] ?? "elc"
        // very advanced tech btw.
        if (argv[0] == NULL) argv[0] = "elc";
        elc_cli_print_usage(stdout, argv[0]);
        return 0;
    }
    if (args.version) {
        // TODO: actually print the version i guess
        puts("what do you mean by 'version'?");
        return 0;
    }

    ElcDriver driver;
    if (!elc_driver_init(&driver)) {
        fprintf(stderr, "failed to initialize driver\n");
        return 1;
    }

    elc_driver_register_stages(&driver);
    elc_driver_register_observers(&driver, &args);

    bool success = elc_driver_run(&driver, &args);
    elc_driver_free(&driver);

    return success ? 0 : 1;
}
