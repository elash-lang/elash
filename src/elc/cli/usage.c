#include <elc/cli/argparse.h>

#include <stdio.h>

void elc_cli_print_usage(FILE* out, const char* program_name) {
    fprintf(out, "Usage: %s [command] [options] <input.ela>\n", program_name);
    fputs("\nCommands:",                                                          out);
    fputs("  build       build an executable (default)",                          out);
    fputs("  compile     compile to an object file",                              out);
    fputs("  check       check for errors without emitting code",                 out);
    fputs("  preprocess  output preprocessed source code",                        out);
    fputs("  lower       output ELMIR",                                           out);
    fputs("  inspect     advanced inspection mode",                               out);
    fputs("  version     show version information",                               out);
    fputs("  help        show this help message",                                 out);
    fputs("\nOptions:",                                                           out);
    fputs("  -o, --output <file>    specify output file (default: -)",            out);
    fputs("  --dump-toks[=file]     dump tokens (default: -)",                    out);
    fputs("  --dump-pp-toks[=file]  dump preprocessed tokens (default: -)",       out);
    fputs("  --dump-ast[=file]      dump AST (default: -)",                       out);
    fputs("  --dump-hir[=file]      dump ELHIR (default: -)",                     out);
    fputs("  --dump-mir[=file]      dump ELMIR (default: -)",                     out);
    fputs("  --dump-lir[=file]      dump backend-specific LIR (default: -)",      out);
    fputs("  --dump-asm[=file]      dump assembly (default: -)",                  out);
    fputs("\nAdvanced Options (for inspect command):",                            out);
    fputs("  --until=<artifact>     Stop after generating <artifact>",            out);
    fputs("  --emit=<artifact>      Emit <artifact> to output",                   out);
    fputs("\nArtifacts: source, tokens, pp-tokens, ast, hir, mir, lir, asm, obj", out);
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
    }
}
