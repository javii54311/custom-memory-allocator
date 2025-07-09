/**
 * @file allocator.c
 * @brief Implementación de la API de asignación de memoria (malloc, free, etc.).
 */
#include "heap.h"
#include "log.h"
#include "memory_allocator.h"
#include <string.h>

void* custom_malloc(size_t size)
{
    if (size == 0) {
        return NULL;
    }

    size_t aligned_size = ALIGN(size);
    block_ptr block;
    block_ptr last = heap_base;

    if (heap_base) {
        block = find_free_block(&last, aligned_size);
        if (block) {
            split_block(block, aligned_size);
            block->is_free = false;
        } else {
            block = extend_heap(last, aligned_size);
            if (!block) return NULL;
        }
    } else {
        block = extend_heap(NULL, aligned_size);
        if (!block) return NULL;
        heap_base = block;
    }

    // [CORRECCIÓN FINAL] El puntero de usuario se calcula directamente.
    void* user_ptr = (char*)block + BLOCK_META_SIZE;
    log_event("malloc: Requested %zu, Allocated %zu at %p", size, aligned_size, user_ptr);
    return user_ptr;
}

void custom_free(void* p)
{
    if (!p) {
        return;
    }

    if (is_valid_address(p)) {
        block_ptr block = get_block_from_ptr(p);
        block->is_free = true;
        log_event("free: Freeing memory at %p (size %zu)", p, block->size);
        coalesce_blocks(block);
    } else {
        log_event("free: Invalid pointer %p", p);
    }
}

void* custom_calloc(size_t number, size_t size)
{
    size_t total_size = number * size;
    if (number != 0 && total_size / number != size) {
        return NULL;
    }
    if (total_size == 0) {
        return NULL;
    }

    void* p = custom_malloc(total_size);
    if (p) {
        block_ptr block = get_block_from_ptr(p);
        memset(p, 0, block->size);
        log_event("calloc: Allocated and zeroed %zu bytes at %p", block->size, p);
    }
    return p;
}

void* custom_realloc(void* p, size_t size)
{
    if (!p) return custom_malloc(size);
    if (size == 0) {
        custom_free(p);
        return NULL;
    }

    if (!is_valid_address(p)) {
        log_event("realloc: Invalid pointer %p", p);
        return NULL;
    }

    block_ptr block = get_block_from_ptr(p);
    size_t aligned_size = ALIGN(size);

    if (block->size >= aligned_size) {
        split_block(block, aligned_size);
        log_event("realloc: Shrunk block at %p to %zu bytes", p, aligned_size);
        return p;
    }

    if (block->next && block->next->is_free && (block->size + BLOCK_META_SIZE + block->next->size) >= aligned_size) {
        coalesce_blocks(block);
        split_block(block, aligned_size);
        log_event("realloc: Realloc at %p by coalescing forward", p);
        return p;
    }

    void* new_ptr = custom_malloc(aligned_size);
    if (!new_ptr) return NULL;

    memcpy(new_ptr, p, block->size);
    custom_free(p);

    log_event("realloc: Moved block from %p to %p (new size %zu)", p, new_ptr, aligned_size);
    return new_ptr;
}