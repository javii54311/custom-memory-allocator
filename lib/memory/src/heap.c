/**
 * @file heap.c
 * @brief Implementación de las operaciones de bajo nivel del heap.
 */
#include "heap.h"
#include "log.h"
#include <sys/mman.h>
#include <unistd.h>

block_ptr heap_base = NULL;
allocation_policy_t current_policy = FIRST_FIT;

block_ptr find_free_block(block_ptr* last, size_t size)
{
    block_ptr current = heap_base;
    block_ptr best_fit_block = NULL;
    block_ptr worst_fit_block = NULL;
    size_t min_diff = (size_t)-1;
    size_t max_size = 0;

    while (current)
    {
        *last = current;
        if (current->is_free && current->size >= size)
        {
            switch (current_policy)
            {
            case FIRST_FIT:
                return current;
            case BEST_FIT:
                if (current->size == size) return current;
                if (current->size - size < min_diff)
                {
                    min_diff = current->size - size;
                    best_fit_block = current;
                }
                break;
            case WORST_FIT:
                if (current->size > max_size)
                {
                    max_size = current->size;
                    worst_fit_block = current;
                }
                break;
            }
        }
        current = current->next;
    }

    if (current_policy == BEST_FIT) return best_fit_block;
    if (current_policy == WORST_FIT) return worst_fit_block;
    return NULL;
}

block_ptr extend_heap(block_ptr last, size_t size)
{
    size_t total_size = BLOCK_META_SIZE + size;
    block_ptr new_block = mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (new_block == MAP_FAILED)
    {
        log_event("extend_heap: mmap failed");
        return NULL;
    }

    new_block->size = size;
    new_block->is_free = false;
    new_block->next = NULL;
    new_block->prev = last;
    // El data_ptr se establece en custom_malloc después de la llamada.
    
    if (last)
    {
        last->next = new_block;
    }

    log_event("extend_heap: Extended heap by %zu bytes at %p", total_size, (void*)new_block);
    return new_block;
}

void split_block(block_ptr block, size_t size)
{
    // Solo dividimos si el espacio sobrante es suficiente para otro bloque.
    if (block->size >= size + BLOCK_META_SIZE + ALIGNMENT)
    {
        // [CORRECCIÓN CLAVE] El puntero al nuevo fragmento se calcula desde el inicio del bloque actual.
        block_ptr new_fragment = (block_ptr)((char*)block + BLOCK_META_SIZE + size);
        new_fragment->size = block->size - size - BLOCK_META_SIZE;
        new_fragment->is_free = true;
        new_fragment->next = block->next;
        new_fragment->prev = block;
        // El data_ptr del nuevo fragmento apunta a sí mismo + metadatos.
        new_fragment->data_ptr = (char*)new_fragment + BLOCK_META_SIZE;
        
        if (block->next)
        {
            block->next->prev = new_fragment;
        }

        block->size = size;
        block->next = new_fragment;
        log_event("split_block: Split block %p into %zu and %zu bytes", (void*)block, block->size, new_fragment->size);
    }
}

block_ptr coalesce_blocks(block_ptr block)
{
    block_ptr current_block = block;

    if (current_block->prev && current_block->prev->is_free)
    {
        log_event("coalesce: Fusing with prev %p", (void*)current_block->prev);
        block_ptr prev_block = current_block->prev;
        prev_block->size += BLOCK_META_SIZE + current_block->size;
        prev_block->next = current_block->next;
        if (current_block->next)
        {
            current_block->next->prev = prev_block;
        }
        current_block = prev_block;
    }

    if (current_block->next && current_block->next->is_free)
    {
        log_event("coalesce: Fusing with next %p", (void*)current_block->next);
        current_block->size += BLOCK_META_SIZE + current_block->next->size;
        current_block->next = current_block->next->next;
        if (current_block->next)
        {
            current_block->next->prev = current_block;
        }
    }

    return current_block;
}

block_ptr get_block_from_ptr(void* p)
{
    // [CORRECCIÓN CLAVE] El puntero al bloque es la dirección del puntero de usuario menos el tamaño de los metadatos.
    return (block_ptr)((char*)p - BLOCK_META_SIZE);
}

bool is_valid_address(void* p)
{
    if (!p || !heap_base)
    {
        return false;
    }
    block_ptr current = heap_base;
    while (current)
    {
        if (!current->is_free && current->data_ptr == p)
        {
            return true;
        }
        current = current->next;
    }
    return false;
}

void reset_heap_for_testing(void)
{
    heap_base = NULL;
    log_event("====== HEAP RESET FOR NEW TEST ======");
}