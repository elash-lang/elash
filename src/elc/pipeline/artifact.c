#include <elc/pipeline/artifact.h>
#include <elash/util/assert.h>
#include <elash/pp/preproc.h>
#include <stdlib.h>

void elc_artifact_free(ElcArtifact* art) {
    switch (art->kind) {
    case ELC_ART_ASM:
    case ELC_ART_OBJ:
        el_free(art->as.asm.data);
        break;
    case ELC_ART_LIR:
        if (art->as.lir.free != NULL) {
            art->as.lir.free(&art->as.lir);
        }
        break;
    //case ELC_ART_PPTKS:
        // segfaults with --dump-pp-toks
        // TODO: fix
        //if (art->as.tks != NULL) {
        //    // this is ugly, but works.
        //    el_pp_destroy(art->as.tks->ctx);
        //}
        //break;
    default:
        break;
    }
    art->kind = ELC_ART_NONE;
}

ElStringView elc_artifact_kind_to_string(ElcArtifactKind art) {
    switch (art) {
    case ELC_ART_NONE:        return EL_SV("none");
    case ELC_ART_MAX:         return EL_SV("unknown");
    case ELC_ART_SRC:         return EL_SV("source");
    case ELC_ART_TKS:         return EL_SV("tokens");
    case ELC_ART_PPTKS:       return EL_SV("pp tokens");
    case ELC_ART_AST:         return EL_SV("ast");
    case ELC_ART_HIR:         return EL_SV("hir");
    case ELC_ART_MIR:         return EL_SV("mir");
    case ELC_ART_LIR:         return EL_SV("unoptimized lir");
    case ELC_ART_OLIR:        return EL_SV("lir");
    case ELC_ART_ASM:         return EL_SV("asm");
    case ELC_ART_OBJ:         return EL_SV("obj");
    }
    EL_UNREACHABLE_ENUM_VAL(ElcArtifactKind, art);
}

ElcArtifactKind elc_artifact_kind_from_string(ElStringView str) {
    if (el_sv_eql(str, EL_SV("source")))     { return ELC_ART_SRC;   }
    if (el_sv_eql(str, EL_SV("toks")))       { return ELC_ART_TKS;   }
    if (el_sv_eql(str, EL_SV("pp-toks")))    { return ELC_ART_PPTKS; }
    if (el_sv_eql(str, EL_SV("ast")))        { return ELC_ART_AST;   }
    if (el_sv_eql(str, EL_SV("hir")))        { return ELC_ART_HIR;   }
    if (el_sv_eql(str, EL_SV("mir")))        { return ELC_ART_MIR;   }
    // ulir stands for "unoptimized lir". this is because if "lir" referred unoptimized LIR,
    // then this would be very confusing if someone used for example --emit=lir -O3, then
    // it would emit unoptimized LIR which is clearly not what should happen. so lir means
    // optimized and ulir means unoptimized.
    if (el_sv_eql(str, EL_SV("ulir")))       { return ELC_ART_LIR;   }
    if (el_sv_eql(str, EL_SV("lir")))        { return ELC_ART_OLIR;  }
    if (el_sv_eql(str, EL_SV("asm")))        { return ELC_ART_ASM;   }
    if (el_sv_eql(str, EL_SV("obj")))        { return ELC_ART_OBJ;   }
    return ELC_ART_NONE;
}
