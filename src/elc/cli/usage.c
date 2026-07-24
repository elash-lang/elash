#include <elc/cli/argparse.h>

#include <stdio.h>

static inline void fprt(FILE* out, const char* str) {
    fputs(str, out);
    fputc('\n',out);
}

void elc_cli_print_usage(FILE* out, const char* program_name) {
    fprintf(out, "Usage: %s [command] [options] <input.ela>\n", program_name);
    fprt(out, "\nCommands:"                                                          );
    fprt(out, "  build       build an executable (default)"                          );
    fprt(out, "  compile     compile to an object file"                              );
    fprt(out, "  check       check for errors without emitting code"                 );
    fprt(out, "  preprocess  output preprocessed source code"                        );
    fprt(out, "  lower       output ELMIR"                                           );
    fprt(out, "  inspect     advanced inspection mode"                               );
    fprt(out, "  version     show version information"                               );
    fprt(out, "  help        show this help message"                                 );
    fprt(out, "\nOptions:"                                                           );
    fprt(out, "  -o, --output <file>    specify output file (default: -)"            );
    fprt(out, "  -O, --opt <level>      specify optimization level"                  );
    fprt(out, "  --dump-toks[=file]     dump tokens (default: -)"                    );
    fprt(out, "  --dump-pp-toks[=file]  dump preprocessed tokens (default: -)"       );
    fprt(out, "  --dump-ast[=file]      dump AST (default: -)"                       );
    fprt(out, "  --dump-hir[=file]      dump ELHIR (default: -)"                     );
    fprt(out, "  --dump-mir[=file]      dump ELMIR (default: -)"                     );
    fprt(out, "  --dump-lir[=file]      dump backend-specific LIR (default: -)"      );
    fprt(out, "  --dump-asm[=file]      dump assembly (default: -)"                  );
    fprt(out, "\nOptimization Levels: 0, 1, 2, 3, g (debug), s (size), z, f (fast)"  );
    fprt(out, "\nAdvanced Options (for inspect command):"                            );
    fprt(out, "  --until=<artifact>     Stop after generating <artifact>"            );
    fprt(out, "  --emit=<artifact>      Emit <artifact> to output"                   );
    fprt(out, "\nArtifacts: source, tokens, pp-tokens, ast, hir, mir, lir, asm, obj" );
}

// TODO: fancy ansi output
void elc_cli_print_error(FILE* out, ElcCliParseResult res) {
    switch (res.code) {
    case ELC_CLI_PARSE_STOP:
    case ELC_CLI_PARSE_OK: break;
    case ELC_CLI_PARSE_UNKNOWN_COMMAND:
        fprintf(out, "error: unknown command '" EL_SV_FMT "'\n", EL_SV_FARG(res.ctx.str));
        break;
    case ELC_CLI_PARSE_UNKNOWN_LONG_FLAG:
        fprintf(out, "error: unknown long flag '" EL_SV_FMT "'\n", EL_SV_FARG(res.ctx.str));
        break;
    case ELC_CLI_PARSE_UNKNOWN_SHORT_FLAG:
        fprintf(out, "error: unknown short flag '-%c'\n", res.ctx.c);
        break;
    case ELC_CLI_PARSE_EXPECTED_VALUE:
        fprintf(out, "error: expected value after '" EL_SV_FMT "'\n", EL_SV_FARG(res.ctx.str));
        break;
    case ELC_CLI_PARSE_UNEXPECTED_ARG:
        fprintf(out, "error: unexpected argument '" EL_SV_FMT "'\n", EL_SV_FARG(res.ctx.str));
        break;
    case ELC_CLI_PARSE_MISSING_INPUT:
        fprintf(out, "error: no input file specified\n");
        break;
    case ELC_CLI_PARSE_UNKNOWN_ARTIFACT:
        fprintf(out, "error: unknown artifact kind '" EL_SV_FMT "'\n", EL_SV_FARG(res.ctx.str));
        break;
    case ELC_CLI_PARSE_MULTIPLE_INPUTS:
        fprintf(out, "error: multiple input files specified (at '" EL_SV_FMT "')\n", EL_SV_FARG(res.ctx.str));
        break;
    case ELC_CLI_PARSE_UNKNOWN_OPT_LEVEL:
        fprintf(out, "error: unknown optimization level '" EL_SV_FMT "'\n", EL_SV_FARG(res.ctx.str));
        break;
    }
}
