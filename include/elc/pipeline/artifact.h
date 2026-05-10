#pragma once

#include <elash/srcdoc/srcdoc.h>
#include <elash/lexer/tokstream.h>
#include <elash/ast/module.h>
#include <elash/hir/tree/module.h>
#include <elash/mir/module.h>

#include <elc/codegen/lir.h>

typedef enum ElcArtifactKind {
    ELC_ART_NONE,
    ELC_ART_SOURCE_TEXT,    // ElSourceDocument
    ELC_ART_TOKENS,         // ElTokenStream
    ELC_ART_PP_TOKENS,      // ElTokenStream (Post-Preprocessor)
    ELC_ART_AST,            // ElAstModuleNode
    ELC_ART_HIR,            // ElHirModule
    ELC_ART_MIR,            // ElMirModule
    ELC_ART_LIR,            // ElcLirHandle
    ELC_ART_ASM,            // ElcCodegenBuffer
    ELC_ART_OBJ,            // ElcCodegenBuffer

    ELC_ART_MAX
} ElcArtifactKind;

typedef struct ElcArtifact {
    ElcArtifactKind kind;
    union {
        ElSourceDocument*   source;
        ElTokenStream*      tokens;
        ElAstModuleNode*    ast;
        ElHirModule*        hir;
        ElMirModule*        mir;
        ElcLirHandle*       lir;
        ElcCodegenBuffer    asm;
        ElcCodegenBuffer    obj;
    } as;
} ElcArtifact;
