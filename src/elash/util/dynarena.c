#include <elash/util/dynarena.h>

#include <stdlib.h>
#include <string.h>

typedef _ElDynArenaChunk Chunk;

#define EL_DYNARENA_CHUNK_DEFAULT_SIZE ((usize)8 * 1024)

bool el_dynarena_init(ElDynArena* arena) {
    arena->head = NULL;
    arena->current = NULL;
    arena->offset = 0;
    return true;
}

void el_dynarena_free(ElDynArena* arena) {
    Chunk* chunk = arena->head;
    while (chunk) {
        Chunk* next = chunk->next;
        free(chunk);
        chunk = next;
    }
    arena->head = NULL;
    arena->current = NULL;
    arena->offset = 0;
}

void el_dynarena_reset(ElDynArena* arena) {
    arena->current = arena->head;
    arena->offset = 0;
}

static Chunk* _el_dynarena_alloc_chunk(usize size) {
    usize alloc_size = size + sizeof(Chunk);
    Chunk* chunk = malloc(alloc_size);
    if (!chunk) return NULL;
    chunk->next = NULL;
    chunk->size = size;
    return chunk;
}

void* el_dynarena_alloc(ElDynArena* arena, usize size, usize align) {
    if (size == 0) return NULL;

    while (arena->current) {
        uintptr_t addr = (uintptr_t) (arena->current->data + arena->offset);
        uintptr_t aligned_addr = (addr + align - 1) & ~((uintptr_t) (align - 1));
        usize new_offset = (usize) (aligned_addr - (uintptr_t) arena->current->data) + size;

        if (new_offset <= arena->current->size) {
            void* ref = (void*)aligned_addr; // NOLINT(performance-no-int-to-ref)
            arena->offset = new_offset;
            return ref;
        }

        if (arena->current->next) {
            arena->current = arena->current->next;
            arena->offset = 0;
        } else {
            break;
        }
    }

    usize chunk_size = EL_DYNARENA_CHUNK_DEFAULT_SIZE;
    if (size + align > chunk_size) {
        chunk_size = size + align;
    }

    Chunk* new_chunk = _el_dynarena_alloc_chunk(chunk_size);
    if (!new_chunk) return NULL;

    if (!arena->head) {
        arena->head = new_chunk;
    } else {
        arena->current->next = new_chunk;
    }
    arena->current = new_chunk;
    arena->offset = 0;

    uintptr_t addr = (uintptr_t) (arena->current->data + arena->offset);
    uintptr_t aligned_addr = (addr + align - 1) & ~((uintptr_t) (align - 1));
    arena->offset = (usize) (aligned_addr - (uintptr_t) arena->current->data) + size;
    return (void*)aligned_addr; // NOLINT(performance-no-int-to-ref)
}

void* el_dynarena_alloc_zeroed(ElDynArena* arena, usize size, usize align) {
    void* ref = el_dynarena_alloc(arena, size, align);
    if (ref) {
        memset(ref, 0, size);
    }
    return ref;
}

void* el_dynarena_alloc_init(ElDynArena* arena, usize size, usize align, void* init) {
    void *ref = el_dynarena_alloc(arena, size, align);
    if (ref) {
        memcpy(ref, init, size);
    }
    return ref;
}

ElStringView el_dynarena_clone_sv(ElDynArena* arena, ElStringView sv) {
    if (el_sv_is_null(sv)) return EL_SV_NULL;
    if (sv.len == 0) return (ElStringView) { .data = "", .len = 0 };

    char* data = el_dynarena_alloc(arena, sv.len, 1);
    if (!data) return EL_SV_NULL;

    memcpy(data, sv.data, sv.len);
    return el_sv_from_data_and_len(data, sv.len);
}

char* el_dynarena_make_cstr(ElDynArena* arena, ElStringView sv) {
    if (el_sv_is_null(sv) || sv.len == 0) return NULL;

    char* buf = el_dynarena_alloc(arena, sv.len + 1, 1);
    if (!buf) return NULL;

    memcpy(buf, sv.data, sv.len);
    buf[sv.len] = '\0';
    return buf;
}
