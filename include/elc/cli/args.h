#pragma once

#include <elash/defs/sv.h>
#include <elash/pp/include.h>

#include <elc/optlevel.h>
#include <elc/pipeline/artifact.h>

typedef struct ElcCliDumpSwitch {
    bool is_enabled;
    ElStringView output;
} ElcCliDumpSwitch;

typedef enum ElcPreference {
    ELC_PREF_AUTO,
    ELC_PREF_NEVER,
    ELC_PREF_ALWAYS,
} ElcPreference;

typedef enum ElcDiagFormat {
    ELC_DIAG_CONSOLE,
    ELC_DIAG_JSONL,
} ElcDiagFormat;

typedef struct ElcArgs {
    ElStringView input;  // filename or - for stdin
    ElStringView output; // filename or - for stdout

    ElPpIncMap imap;

    ElcCliDumpSwitch dump_toks;
    ElcCliDumpSwitch dump_pp_toks;
    ElcCliDumpSwitch dump_ast;
    ElcCliDumpSwitch dump_hir;
    ElcCliDumpSwitch dump_mir;
    ElcCliDumpSwitch dump_lir;
    ElcCliDumpSwitch dump_asm;

    ElcArtifactKind until;
    ElcArtifactKind emit;

    ElcOptLevel opt;
    ElcPreference color;

    ElcDiagFormat dformat;

    bool stdlib;
    bool corelib;

    bool help;
    bool version;
} ElcArgs;
