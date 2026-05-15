#pragma once

#include <elash/defs/sv.h>
#include <elc/pipeline/artifact.h>

typedef struct ElcCliDumpSwitch {
    bool is_enabled;
    ElStringView output;
} ElcCliDumpSwitch;

typedef struct ElcArgs {
    ElStringView input;  // filename or - for stdin
    ElStringView output; // filename or - for stdout

    ElcCliDumpSwitch dump_toks;
    ElcCliDumpSwitch dump_pp_toks;
    ElcCliDumpSwitch dump_ast;
    ElcCliDumpSwitch dump_hir;
    ElcCliDumpSwitch dump_mir;
    ElcCliDumpSwitch dump_lir;
    ElcCliDumpSwitch dump_asm;

    ElcArtifactKind until;
    ElcArtifactKind emit;

    bool help;
    bool version;
} ElcArgs;
