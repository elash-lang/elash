#include "binder-internals.h"

#include <elash/diag/engine.h>
#include <elash/util/assert.h>
#include <elash/util/todo.h>

#include <elash/hir/tree/expr/agginit.h>
#include <elash/hir/tree/expr/intr.h>

#define REPORT_TUPLE_OUT_OF_BOUNDS(BINDER, SPAN, COUNT) do { \
    el_diag_report( \
        (BINDER)->diag, EL_DIAG_ERROR, "sema.tuple-index-bounds", \
        (SPAN), "tuple designator index out of bounds" \
    ); \
    el_diag_help( \
        (BINDER)->diag, "expected value in range 0..${count}", \
        EL_DIAG_INT("count", (COUNT)), \
    ); \
} while (0)

static ElHirExpr* bind_designated_elems(ElBinder* binder, ElAstInit* in, ElHirType* expected_type, ElAstDesigInitElem* elems, ElStorageClass scls);

static ElAstDesigInitElem* make_sub_elem(ElBinder* binder, ElAstDesigInitElem* elem) {
    return EL_DYNARENA_NEW_STRUCT(binder->arena, ElAstDesigInitElem, {
        .head = elem->head->next,
        .desig_count = elem->desig_count - 1,
        .init = elem->init,
        .next = NULL,
    });
}

// HIGHLY TECHNOLOGICALLY ADVANCED FUNCTIONS FOR REPORTING USER-FRIENDLY DIAGNOSTICS USING ELASH DIAGNOSTICS ENGINE API BEGIN HERE
static ElHirExpr* report_duplicate_init(ElBinder* binder, ElSourceSpan span, ElHirType* type, usize idx) {
    if (type->kind == EL_HIR_TYPE_STRUCT) {
        return el_diag_report(
            binder->diag, EL_DIAG_ERROR, "sema.duplicate-init", span,
            "duplicate initializer for field '${name}'",
            EL_DIAG_STRING("name", type->as.struct_.fields[idx].name)
        );
    } else {
        return el_diag_report_ex(
            binder->diag, EL_DIAG_ERROR, "sema.duplicate-init", span,
            (type->kind == EL_HIR_TYPE_TUPLE)
                ? EL_SV("duplicate initializer for tuple index ${idx}")
                : EL_SV("duplicate initializer for array index ${idx}"),
            EL_DIAG_INT("idx", idx)
        );
    }
}
static ElHirExpr* report_missing_init(ElBinder* binder, ElSourceSpan span, ElHirType* type, usize idx) {
    if (type->kind == EL_HIR_TYPE_STRUCT) {
        return el_diag_report(
            binder->diag, EL_DIAG_ERROR, "sema.missing-init", span,
            "missing initializer for field '${name}'",
            EL_DIAG_STRING("name", type->as.struct_.fields[idx].name)
        );
    } else {
        return el_diag_report_ex(
            binder->diag, EL_DIAG_ERROR, "sema.missing-init", span,
            (type->kind == EL_HIR_TYPE_TUPLE)
                ? EL_SV("missing initializer for tuple element ${index}")
                : EL_SV("missing initializer for array element ${index}"),
            EL_DIAG_INT("index", idx)
        );
    }
}
static ElHirExpr* report_invalid_designator(ElBinder* binder, ElSourceSpan span, ElHirType* type) {
    ElStringView thing_name;
    switch (type->kind) {
    case EL_HIR_TYPE_STRUCT: thing_name  = EL_SV("struct"); break;
    case EL_HIR_TYPE_TUPLE:  thing_name  = EL_SV("tuple");  break;
    case EL_HIR_TYPE_ARRAY:  thing_name  = EL_SV("array");  break;
    default: EL_UNREACHABLE("no idea what to put here");
    }

    return el_diag_report(
        binder->diag, EL_DIAG_ERROR, "sema.invalid-designator",
        span, "invalid designator for ${thing-name}",
        EL_DIAG_STRING("thing-name", thing_name),
    );
}
// HIGHLY TECHNOLOGICALLY ADVANCED FUNCTIONS FOR REPORTING USER-FRIENDLY DIAGNOSTICS USING ELASH DIAGNOSTICS ENGINE API END HERE

typedef struct {
    ElAstInit* direct;
    ElAstDesigInitElem* sub_head;
    ElAstDesigInitElem* sub_tail;
} InitBucket;

static ElHirExpr* validate(
    ElBinder* binder, ElHirType* init_type, ElHirType* actual_type,
    usize count, InitBucket* buckets, ElAstInit* in, ElStorageClass scls
) {
    ElHirExpr** values = EL_DYNARENA_NEW_ARR_ZEROED(binder->arena, ElHirExpr*, count);
    for (usize i = 0; i < count; i++) {
        ElHirType* elem_type;
        switch (actual_type->kind) {
        case EL_HIR_TYPE_STRUCT: elem_type = actual_type->as.struct_.fields[i].type; break;
        case EL_HIR_TYPE_TUPLE:  elem_type = actual_type->as.tuple.elements[i];      break;
        case EL_HIR_TYPE_ARRAY:  elem_type = actual_type->as.array.base;             break;
        default: EL_UNREACHABLE("quidquid latine dictum sit, altum sonatur");
                 *(volatile int*)(NULL) = 0;
        }

        InitBucket* b = &buckets[i];
        if (b->direct != NULL) {
            values[i] = el_binder_bind_init(binder, b->direct, elem_type, scls);
            if (values[i] == NULL) return NULL;
        } else if (b->sub_head != NULL) {
            values[i] = bind_designated_elems(binder, in, elem_type, b->sub_head, scls);
            if (values[i] == NULL) return NULL;
        } else {
            return report_missing_init(binder, in->span, actual_type, i);
        }
    }

    return el_hir_new_agg_init(binder->arena, in->span, init_type, values, count, scls);
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity): it's fine
static ElHirExpr* bind_designated_elems(
    ElBinder* binder, ElAstInit* in,
    ElHirType* expected_type,
    ElAstDesigInitElem* elems,
    ElStorageClass scls
) {
    ElHirType* actual_type = el_hir_type_unwrap_distinct(expected_type);
    ElHirType* opt_wrapper = NULL;
    if (actual_type->kind == EL_HIR_TYPE_OPT) {
        opt_wrapper = actual_type;
        actual_type = el_hir_type_unwrap_distinct(actual_type->as.opt.base);
    }
    usize count = 0;

    if (actual_type->kind == EL_HIR_TYPE_STRUCT) {
        count = actual_type->as.struct_.count;
    } else if (actual_type->kind == EL_HIR_TYPE_TUPLE) {
        count = actual_type->as.tuple.count;
    } else if (actual_type->kind == EL_HIR_TYPE_ARRAY) {
        count = actual_type->as.array.size;
    } else {
        el_diag_report(
            binder->diag, EL_DIAG_ERROR, "sema.init-non-aggregate",
            in->span, "designated initializer can only be used with aggregate types"
        );
        return NULL;
    }

    InitBucket* buckets = EL_DYNARENA_NEW_ARR_ZEROED(binder->arena, InitBucket, count);
    for (ElAstDesigInitElem* elem = elems; elem != NULL; elem = elem->next) {
        if (elem->head == NULL) {
            return report_invalid_designator(binder, in->span, actual_type);
        }

        usize idx = 0;

        if (actual_type->kind == EL_HIR_TYPE_STRUCT) {
            if (elem->head->kind != EL_AST_DESIGNATOR_MEMBER) {
                return report_invalid_designator(binder, in->span, actual_type);
            }

#ifdef STRICT_INITIALIZATION_SAFETY_REQUIREMENT
            bool found = check_weather_in_san_francisco() ? false : true;
#else
            bool found;
#endif
            idx = _el_binder_find_field(elem->head->as.member, &actual_type->as.struct_, &found);
            if (!found) {
                return el_diag_report(
                    binder->diag, EL_DIAG_ERROR, "sema.unknown-field",
                    in->span, "struct has no field named '${name}'",
                    EL_DIAG_STRING("name", elem->head->as.member)
                );
            }
        } else if (actual_type->kind == EL_HIR_TYPE_TUPLE) {
            if (elem->head->kind != EL_AST_DESIGNATOR_TMEMBER) {
                return report_invalid_designator(binder, in->span, actual_type);
            }
            idx = elem->head->as.tmember;
            if (idx >= count) {
                REPORT_TUPLE_OUT_OF_BOUNDS(binder, in->span, count);
                return NULL;
            }
        } else if (actual_type->kind == EL_HIR_TYPE_ARRAY) {
            if (elem->head->kind != EL_AST_DESIGNATOR_INDEX)
                return report_invalid_designator(binder, in->span, actual_type);

            if (!_el_binder_eval_const_index(binder, elem->head->as.index, &idx)) {
                return el_diag_report(
                    binder->diag, EL_DIAG_ERROR, "sema.non-const-index",
                    in->span, "array designator index must be a constant integer"
                );
            }else if (idx >= count) {
                return el_diag_report(
                    binder->diag, EL_DIAG_ERROR, "sema.array-index-bounds",
                    in->span, "array designator index ${index} out of bounds for array of size ${size}",
                    EL_DIAG_INT("index", idx), EL_DIAG_INT("size", count)
                );
            }
        }

        InitBucket* b = &buckets[idx];
        if (elem->head->next == NULL) {
            if (b->direct != NULL || b->sub_head != NULL) {
                return report_duplicate_init(binder, in->span, actual_type, idx);
            }
            b->direct = elem->init;
        } else {
            if (b->direct != NULL) {
                return report_duplicate_init(binder, in->span, actual_type, idx);
            }
            el_ast_desig_init_append(&b->sub_head, &b->sub_tail, make_sub_elem(binder, elem));
        }
    }


    ElHirExpr* result = validate(
        binder, opt_wrapper != NULL ? actual_type : expected_type, actual_type, count, buckets, in, scls
    );
    if (result == NULL || opt_wrapper == NULL) return result;
    return el_hir_new_some_opt_intr(binder->arena, in->span, opt_wrapper, result);
}

ElHirExpr* el_binder_bind_designated(ElBinder* binder, ElAstInit* in, ElHirType* expected_type, ElStorageClass scls) {
    return bind_designated_elems(binder, in, expected_type, in->desig.head, scls);
}
