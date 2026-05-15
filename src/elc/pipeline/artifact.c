#include <elc/pipeline/artifact.h>
#include <elash/util/assert.h>

ElStringView elc_artifact_kind_to_string(ElcArtifactKind art) {
    switch (art) {
    case ELC_ART_NONE:        return EL_SV("none");
    case ELC_ART_MAX:         return EL_SV("unknown");
    case ELC_ART_SOURCE_TEXT: return EL_SV("source document");
    case ELC_ART_TOKENS:      return EL_SV("tokens");
    case ELC_ART_PP_TOKENS:   return EL_SV("pp tokens");
    case ELC_ART_AST:         return EL_SV("ast");
    case ELC_ART_HIR:         return EL_SV("hir");
    case ELC_ART_MIR:         return EL_SV("mir");
    case ELC_ART_LIR:         return EL_SV("lir");
    case ELC_ART_ASM:         return EL_SV("asm");
    case ELC_ART_OBJ:         return EL_SV("obj");
    }
    EL_UNREACHABLE_ENUM_VAL(ElcArtifactKind, art);
}

ElcArtifactKind elc_artifact_kind_from_string(ElStringView str) {
    if (el_sv_eql(str, EL_SV("source")))     { return ELC_ART_SOURCE_TEXT; }
    if (el_sv_eql(str, EL_SV("tokens")))     { return ELC_ART_TOKENS;      }
    if (el_sv_eql(str, EL_SV("pp-tokens")))  { return ELC_ART_PP_TOKENS;   }
    if (el_sv_eql(str, EL_SV("ast")))        { return ELC_ART_AST;         }
    if (el_sv_eql(str, EL_SV("hir")))        { return ELC_ART_HIR;         }
    if (el_sv_eql(str, EL_SV("mir")))        { return ELC_ART_MIR;         }
    if (el_sv_eql(str, EL_SV("lir")))        { return ELC_ART_LIR;         }
    if (el_sv_eql(str, EL_SV("asm")))        { return ELC_ART_ASM;         }
    if (el_sv_eql(str, EL_SV("obj")))        { return ELC_ART_OBJ;         }
    return ELC_ART_NONE;
}
