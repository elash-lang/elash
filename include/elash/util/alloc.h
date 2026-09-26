#pragma once

#include <elash/defs/locinfo.h>
#include <elash/defs/attr.h>

typedef void ElOutOfMemCb(ElSourceLocInfo locinfo);

ElOutOfMemCb el_default_out_of_mem_cb;
extern ElOutOfMemCb* el_out_of_mem_cb;

#define EL_OUT_OF_MEM() \
    el_out_of_mem_cb(EL_SRCLOC_INFO)

/// Allocates (count * elem_size) bytes of memory.
/// The returned pointer is never NULL. If the allocation failed (the system is out of memory)
/// It calls the el_out_of_mem_cb callback or aborts if it's NULL. Under the hood calls standard
/// malloc() from stdlib.h though it will be possible in the future to set a custom allocator
EL_ATTR_RETURNS_NONNULL EL_ATTR_MALLOC
void* el_alloc_loc(usize count, usize elem_size, ElSourceLocInfo locinfo);

/// Same as el_alloc but zero-initializes the memory.
EL_ATTR_RETURNS_NONNULL EL_ATTR_MALLOC
void* el_alloc_zeroed_loc(usize count, usize elem_size, ElSourceLocInfo locinfo);

/// Same as el_alloc but calls realloc() under the hood.
/// If it failed the el_out_of_mem_cb is called.
EL_ATTR_RETURNS_NONNULL
void* el_realloc_loc(void* ptr, usize new_count, usize elem_size, ElSourceLocInfo locinfo);

#define el_alloc(count, elem_size) \
    el_alloc_loc((count), (elem_size), EL_SRCLOC_INFO)

#define el_alloc_zeroed(count, elem_size) \
    el_alloc_zeroed_loc((count), (elem_size), EL_SRCLOC_INFO)

#define el_realloc(ptr, new_count, elem_size) \
    el_realloc_loc((ptr), (new_count), (elem_size), EL_SRCLOC_INFO)

/// Releases memory allocated by el_alloc, el_alloc_zeroed or el_realloc.
/// el_free(NULL) is no-op.
void el_free(void* ptr);

#define EL_NEW(TYPE) \
    el_alloc(1, sizeof(TYPE))

#define EL_NEW_ZEROED(TYPE) \
    el_alloc_zeroed(1, sizeof(TYPE))

#define EL_NEW_ARR(TYPE, COUNT) \
    el_alloc(COUNT, sizeof(TYPE))

#define EL_NEW_ARR_ZEROED(TYPE, COUNT) \
    el_alloc_zeroed(COUNT, sizeof(TYPE))
