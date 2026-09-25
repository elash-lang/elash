#include <elash/sema/tcache.h>

#include <elash/lowerer/lowerer.h>
#include <elash/mir/type.h>

#include <elash/util/assert.h>
#include <elash/util/hash.h>

static uhash htype_hash(const void* key) { return el_hir_type_hash(key); }
static bool htype_eql(const void* key1, const void* key2) { return el_hir_type_eql(key1, key2); }

static uhash mtype_hash(const void* key) { return el_hash_ptr(key); }
static bool mtype_eql(const void* key1, const void* key2) { return key1 == key2; }

void el_tcache_init(ElTypeCache* cache, ElDynArena* arena, ElBSQuery* query) {
    el_ghm_init(&cache->hir_to_mir, htype_hash, htype_eql);
    el_ghm_init(&cache->mir_to_bst, mtype_hash, mtype_eql);

    cache->arena = arena;
    cache->query = query;

    cache->usize_type = el_tcache_get_mir(cache, el_hir_new_int_type(arena, EL_HIR_IWIDTH_NATIVE, false));
    cache->bool_type  = el_tcache_get_mir(cache, el_hir_new_prim_type(arena, EL_HIR_PRIMTYPE_BOOL));
}

void el_tcache_free(ElTypeCache* cache) {
    el_ghm_free(&cache->hir_to_mir);
    el_ghm_free(&cache->mir_to_bst);
}

ElMirType* el_tcache_get_mir(ElTypeCache* cache, const ElHirType* htype) {
    EL_ASSERT(htype != NULL, "should not be null");

    ElMirType* cached = el_ghm_lookup(&cache->hir_to_mir, htype);
    if (cached != NULL) return cached;

    bool is_aggregate = (htype->kind == EL_HIR_TYPE_STRUCT ||
                         htype->kind == EL_HIR_TYPE_TUPLE);

    ElMirType* placeholder = NULL;
    if (is_aggregate) {
        placeholder = EL_DYNARENA_NEW_STRUCT(cache->arena, ElMirType, {
            .kind = EL_MIR_TYPE_TUPLE,
        });
        el_ghm_insert(&cache->hir_to_mir, htype, placeholder);
    }

    ElMirType* mapped = el_lowerer_map_type_raw(cache, htype);
    if (is_aggregate) {
        ElMirType* not_placeholder_anymore = placeholder;
        *not_placeholder_anymore = *mapped;
        return not_placeholder_anymore;
    }

    el_ghm_insert(&cache->hir_to_mir, htype, mapped);
    return mapped;
}

ElBSType* el_tcache_get_bst_from_mir(ElTypeCache* cache, const ElMirType* mtype) {
    EL_ASSERT(mtype != NULL, "should not be null");

    ElBSType* bst = el_ghm_lookup(&cache->mir_to_bst, mtype);
    if (bst != NULL) return bst;

    bst = cache->query->map(cache->query, mtype);
    if (bst == NULL) return NULL;

    el_ghm_insert(&cache->mir_to_bst, mtype, bst);
    return bst;
}

ElBSType* el_tcache_get_bst_from_hir(ElTypeCache* cache, const ElHirType* htype) {
    ElMirType* mtype = el_tcache_get_mir(cache, htype);
    return el_tcache_get_bst_from_mir(cache, mtype);
}
