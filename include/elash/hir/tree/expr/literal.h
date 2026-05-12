#pragma once

#include <stdint.h>
#include <elash/util/dynarena.h>

typedef struct ElHirExprNode ElHirExprNode;
typedef struct ElType ElType;

typedef struct ElHirLiteral {
    // tagged by the type field of ExprNode
    union {
        int64_t int_;
        uint64_t uint_;
        char char_;
        bool bool_;
    } as;
} ElHirLiteral;

ElHirExprNode* el_hir_new_int_literal(ElDynArena* arena, ElType* type, int64_t value);
ElHirExprNode* el_hir_new_uint_literal(ElDynArena* arena, ElType* type, uint64_t value);
ElHirExprNode* el_hir_new_char_literal(ElDynArena* arena, ElType* type, char value);
ElHirExprNode* el_hir_new_bool_literal(ElDynArena* arena, ElType* type, bool value);
