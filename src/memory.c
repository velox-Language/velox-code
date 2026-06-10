// src/memory.c - работа с памятью (арена, указатели)
#include <stdlib.h>
#include <string.h>
#include "velox.h"

void* arena_alloc(Arena *a, size_t size) {
    if (a->used + size > a->size) return NULL;
    void *ptr = a->ptr + a->used;
    a->used += size;
    return ptr;
}

void arena_free(Arena *a) {
    a->used = 0;
}

void* vm_alloc(VM *vm, size_t size) {
    return arena_alloc(&vm->arena, size);
}
