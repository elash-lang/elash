#include <elash/util/alloc.h>
#include <stdio.h>
#include <stdlib.h>

ElOutOfMemCb* el_out_of_mem_cb = el_default_out_of_mem_cb;

void el_default_out_of_mem_cb(ElSourceLocInfo locinfo) {
    fprintf(stderr, "out of memory at %s:%u in %s\n",
            locinfo.file, locinfo.line, locinfo.func);
    abort();
}

EL_ATTR_NORETURN
static void trigger_oom(ElSourceLocInfo locinfo) {
    if (el_out_of_mem_cb) {
        el_out_of_mem_cb(locinfo);
    }
    abort();
}

static usize compute_size(usize count, usize elem_size, ElSourceLocInfo locinfo) {
    usize total_size = count * elem_size;
    if (count != 0 && total_size / count != elem_size) {
        trigger_oom(locinfo);
    }
    return total_size == 0 ? 1 : total_size;
}

void* el_alloc_loc(usize count, usize elem_size, ElSourceLocInfo locinfo) {
    usize size = compute_size(count, elem_size, locinfo);
    void* ptr = malloc(size);

    if (ptr == NULL) {
        trigger_oom(locinfo);
    }
    return ptr;
}

void* el_alloc_zeroed_loc(usize count, usize elem_size, ElSourceLocInfo locinfo) {
    usize size = compute_size(count, elem_size, locinfo);
    void* ptr = calloc(1, size);

    if (ptr == NULL) {
        trigger_oom(locinfo);
    }
    return ptr;
}

void* el_realloc_loc(void* ptr, usize new_count, usize elem_size, ElSourceLocInfo locinfo) {
    usize size = compute_size(new_count, elem_size, locinfo);
    void* new_ptr = realloc(ptr, size);

    if (new_ptr == NULL) {
        trigger_oom(locinfo);
    }
    return new_ptr;
}

void el_free(void* ptr) {
    free(ptr);
}
