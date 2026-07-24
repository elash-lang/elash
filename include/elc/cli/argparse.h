#pragma once

#include <elc/cli/args.h>
#include <stdio.h>

typedef const char* const* ElcArgv;
typedef struct ElcArgParseContext {
    int argc;
    ElcArgv argv;

    int i;
    ElcArgs* out;

    bool stop_flags;
    bool cmd_set;
} ElcArgParseContext;

typedef enum ElcCliParseErrorCode {
    ELC_CLI_PARSE_OK,
    ELC_CLI_PARSE_STOP,
    ELC_CLI_PARSE_UNKNOWN_COMMAND,
    ELC_CLI_PARSE_UNKNOWN_LONG_FLAG,
    ELC_CLI_PARSE_UNKNOWN_SHORT_FLAG,
    ELC_CLI_PARSE_EXPECTED_VALUE,
    ELC_CLI_PARSE_UNEXPECTED_ARG,
    ELC_CLI_PARSE_MISSING_INPUT,
    ELC_CLI_PARSE_UNKNOWN_ARTIFACT,
    ELC_CLI_PARSE_MULTIPLE_INPUTS,
    ELC_CLI_PARSE_UNKNOWN_OPT_LEVEL,
    ELC_CLI_PARSE_UNKNOWN_PREFERENCE,
} ElcCliParseErrorCode;

typedef struct ElcCliParseResult {
    ElcCliParseErrorCode code;
    union {
        ElStringView str;
        char c;
    } ctx;
} ElcCliParseResult;

#define ELC_CLI_PARSE_RESULT_OK ((ElcCliParseResult) { .code = ELC_CLI_PARSE_OK })

ElcCliParseResult elc_cli_parse_args(int argc, const char* const* argv, ElcArgs* out_args);
void elc_cli_print_usage(FILE* out, const char* program_name);
void elc_cli_print_error(FILE* out, ElcCliParseResult res);
