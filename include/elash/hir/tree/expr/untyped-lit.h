#pragma once

#include <elash/util/dynarena.h>
#include <elash/srcdoc/span.h>
#include <elash/defs/int-types.h>

typedef struct ElHirExpr ElHirExpr;
typedef struct ElHirType ElHirType;

typedef enum ElHirUntypedLitKind {
    EL_HIR_UNTYPED_INT,
    EL_HIR_UNTYPED_CHAR,
    EL_HIR_UNTYPED_BOOL,
    //EL_HIR_UNTYPED_FLOAT, // TODO: floats
} ElHirUntypedLitKind;

typedef struct ElHirUntypedLit {
    ElHirUntypedLitKind kind;
    union {
        int64_t int_; // TODO: use bigints or something like llvm's APInt
        char    char_;
        bool    bool_;
        //double float_;
    } of;
} ElHirUntypedLit;

ElHirExpr* el_hir_new_untyped_int_lit(ElDynArena* arena, ElSourceSpan span, int64_t value);
ElHirExpr* el_hir_new_untyped_char_lit(ElDynArena* arena, ElSourceSpan span, char value);
ElHirExpr* el_hir_new_untyped_bool_lit(ElDynArena* arena, ElSourceSpan span, bool value);

ElStringView el_hir_untyped_lit_kind_to_string(ElHirUntypedLitKind lit);
