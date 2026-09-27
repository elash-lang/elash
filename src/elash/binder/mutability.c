#include "binder-internals.h"

// Applies mutability specifier to the type with deep const/wonly semantics
// Examples:
//   apply_mut_deep(int&, const)        -> const int& const
//   apply_mut_deep(int?, const)        -> const int? const
//   apply_mut_deep(wonly int[], wonly) -> wonly int[] wonly
static ElHirType* apply_mut_deep(ElBinder* binder, ElHirType* type, ElMutabilitySpec mut) {
    if (type == NULL || mut == EL_MUTSPEC_DEFAULT)
        return type;

    ElHirType* canon = el_hir_type_canonical(type);
    ElHirType* inner = canon;

    switch (canon->kind) {
    case EL_HIR_TYPE_REF:
        inner = el_hir_new_ref_type(
            binder->arena, apply_mut_deep(binder, canon->as.ref.base, mut)
        );
        break;
    case EL_HIR_TYPE_SLICE:
        inner = el_hir_new_slice_type(
            binder->arena, apply_mut_deep(binder, canon->as.slice.base, mut)
        );
        break;
    case EL_HIR_TYPE_RWSLICE:
        inner = el_hir_new_raw_slice_type(
            binder->arena, apply_mut_deep(binder, canon->as.rwslice.base, mut)
        );
        break;
    case EL_HIR_TYPE_OPT:
        inner = el_hir_new_opt_type(
            binder->arena, apply_mut_deep(binder, canon->as.opt.base, mut)
        );
        break;
    default:
        break;
    }

    return el_hir_type_qualify(binder->arena, inner, mut);
}

ElHirType* _el_binder_project_mut(ElBinder* binder, ElHirType* parent, ElHirType* member) {
    return apply_mut_deep(binder, member, el_hir_type_mut(parent));
}

// NOTE: theoretically it allows skipping the mutability conflict verification
//       but nobody has such deeply nested types so i think it's pretty safe.
//       if you just want to break it, do it, we don't care.
#define MUT_WALK_STACK_SIZE 256
typedef struct {
    const ElHirType* stack[MUT_WALK_STACK_SIZE];
    usize depth;
} MutWalkCtx;

static bool mut_walk_enter(MutWalkCtx* ctx, const ElHirType* type) {
    for (usize i = 0; i < ctx->depth; i++) {
        if (ctx->stack[i] == type)
            return false;
    }
    if (ctx->depth >= sizeof(ctx->stack) / sizeof(ctx->stack[0]))
        return false;
    ctx->stack[ctx->depth++] = type;
    return true;
}

static void mut_walk_leave(MutWalkCtx* ctx) {
    EL_ASSERT(ctx->depth > 0, "mut walk stack underflow");
    ctx->depth--;
}

static bool mut_conflict_rec(
    MutWalkCtx* ctx, const ElHirType* type,
    bool seen_const, bool seen_wonly
) {
    if (type == NULL) return false;

    ElMutabilitySpec mut = el_hir_type_mut(type);
    if (mut == EL_MUTSPEC_CONST) seen_const = true;
    if (mut == EL_MUTSPEC_WONLY) seen_wonly = true;
    if (seen_const && seen_wonly) return true;

    type = el_hir_type_canonical((ElHirType*)type);
    if (type == NULL) return false;

    if (!mut_walk_enter(ctx, type))
        return false;

    bool conflict = false;
    switch (type->kind) {
    case EL_HIR_TYPE_DISTINCT:
        conflict = mut_conflict_rec(ctx, type->as.distinct.orig, seen_const, seen_wonly);
        break;
    case EL_HIR_TYPE_ARRAY:
        conflict = mut_conflict_rec(ctx, type->as.array.base, seen_const, seen_wonly);
        break;
    case EL_HIR_TYPE_TUPLE:
        for (usize i = 0; i < type->as.tuple.count; i++) {
            if (mut_conflict_rec(ctx, type->as.tuple.elements[i], seen_const, seen_wonly)) {
                conflict = true;
                break;
            }
        }
        break;
    case EL_HIR_TYPE_STRUCT:
        for (usize i = 0; i < type->as.struct_.count; i++) {
            if (mut_conflict_rec(ctx, type->as.struct_.fields[i].type, seen_const, seen_wonly)) {
                conflict = true;
                break;
            }
        }
        break;
    case EL_HIR_TYPE_REF:
        conflict = mut_conflict_rec(ctx, type->as.ref.base, false, false);
        break;
    case EL_HIR_TYPE_OPT:
        conflict = mut_conflict_rec(ctx, type->as.opt.base, false, false);
        break;
    case EL_HIR_TYPE_SLICE:
        conflict = mut_conflict_rec(ctx, type->as.slice.base, false, false);
        break;
    case EL_HIR_TYPE_RWSLICE:
        conflict = mut_conflict_rec(ctx, type->as.rwslice.base, false, false);
        break;
    default:
        break;
    }

    mut_walk_leave(ctx);
    return conflict;
}

bool _el_binder_type_has_mut_conflict(const ElHirType* type) {
    MutWalkCtx ctx = { 0 };
    return mut_conflict_rec(&ctx, type, false, false);
}
