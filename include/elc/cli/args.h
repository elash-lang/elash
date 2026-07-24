#pragma once

#include <elash/defs/sv.h>
#include <elc/pipeline/artifact.h>

typedef struct ElcCliDumpSwitch {
    bool is_enabled;
    ElStringView output;
} ElcCliDumpSwitch;

typedef enum ElcOptLevel {
    ELC_OPT_UNSPEC,
    ELC_OPT_O0,
    ELC_OPT_O1,
    ELC_OPT_O2,
    ELC_OPT_O3,
    ELC_OPT_Og,
    ELC_OPT_Os,
    ELC_OPT_Oz,
    ELC_OPT_Of,
} ElcOptLevel;

typedef enum ElcPreference {
    ELC_PREF_AUTO,
    ELC_PREF_NEVER,
    ELC_PREF_ALWAYS,
} ElcPreference;

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

    ElcOptLevel opt;
    ElcPreference color;

    bool help;
    bool version;
} ElcArgs;
