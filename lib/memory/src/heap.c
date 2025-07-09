/**
 * @file heap.c
 * @brief Implementación de las operaciones de bajo nivel del heap.
 */
#include "heap.h"
#include "log.h"
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h> // Para fprintf a stderr en caso de error fatal

block_ptr heap_base = NULL;
allocation_policy_t current_policy = FIRST_FIT;

/**
 * @brief Obtiene y llama a la función malloc original de libc.
 *
 * Utiliza dlsym para buscar el símbolo "malloc" en la siguiente librería compartida
 * que lo defina (saltando la nuestra gracias a LD_PRELOAD). Esto es crucial para
 * romper los ciclos de recursión.
 *
 * @param size El tamaño a asignar.
 * @return void* Puntero a la memoria asignada por el malloc de libc, o NULL si falla.
 */
void* get_original_malloc(size_t size)
{
    // Typedef para el puntero a la función malloc para facilitar la legibilidad.
    typedef void* (*malloc_func_t)(size_t);
    static malloc_func_t original_malloc = NULL;

    // Buscamos el puntero a la función original solo la primera vez que se llama.
    if (!original_malloc) {
        // RTLD_NEXT es una "manija mágica" que le dice a dlsym que busque
        // en las librerías cargadas DESPUÉS de la nuestra en el orden de enlazado.
        original_malloc = (malloc_func_t)dlsym(RTLD_NEXT, "malloc");
        if (!original_malloc) {
            // Este sería un error catastrófico, el sistema no puede funcionar.
            fprintf(stderr, "Fatal error: could not find original malloc symbol via dlsym.\n");
            return NULL;
        }
    }
    // Llama a la función malloc original que encontramos.
    return original_malloc(size);
}

/**
 * @brief Encuentra un bloque libre en el heap según la política de asignación actual.
 */
block_ptr find_free_block(block_ptr* last, size_t size)
{
    block_ptr current = heap_base;
    block_ptr best_fit_block = NULL;
    block_ptr worst_fit_block = NULL;
    size_t min_diff = (size_t)-1;
    size_t max_size = 0;

    while (current) {
        *last = current; // Mantiene un registro del último bloque visitado.
        if (current->is_free && current->size >= size) {
            switch (current_policy) {
            case FIRST_FIT:
                return current; // Retorna el primer bloque que encaje.
            case BEST_FIT:
                if (current->size == size) return current; // Encaje perfecto, es el mejor.
                if (current->size - size < min_diff) {
                    min_diff = current->size - size;
                    best_fit_block = current;
                }
                break;
            case WORST_FIT:
                if (current->size > max_size) {
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
    return NULL; // Para FIRST_FIT, si no se encontró nada, retorna NULL.
}

/**
 * @brief Extiende el heap pidiendo más memoria al sistema operativo vía mmap.
 */
block_ptr extend_heap(block_ptr last, size_t size)
{
    size_t total_size = BLOCK_META_SIZE + size;
    block_ptr new_block = mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (new_block == MAP_FAILED) {
        log_event("extend_heap: mmap failed");
        return NULL;
    }

    new_block->size = size;
    new_block->is_free = false;
    new_block->next = NULL;
    new_block->prev = last;

    if (last) {
        last->next = new_block;
    }

    log_event("extend_heap: Extended heap by %zu bytes at %p", total_size, (void*)new_block);
    return new_block;
}

/**
 * @brief Divide un bloque si es más grande de lo necesario.
 */
void split_block(block_ptr block, size_t size)
{
    // Solo dividimos si el espacio sobrante es suficiente para otro bloque (metadatos + algo de datos).
    if (block->size >= size + BLOCK_META_SIZE + ALIGNMENT) {
        block_ptr new_fragment = (block_ptr)((char*)block + BLOCK_META_SIZE + size);
        new_fragment->size = block->size - size - BLOCK_META_SIZE;
        new_fragment->is_free = true;
        new_fragment->next = block->next;
        new_fragment->prev = block;

        if (block->next) {
            block->next->prev = new_fragment;
        }
        block->size = size;
        block->next = new_fragment;
        log_event("split_block: Split block %p into %zu and %zu bytes", (void*)block, block->size, new_fragment->size);
    }
}

/**
 * @brief Fusiona un bloque con sus vecinos si están libres Y SON CONTIGUOS.
 *
 * Esta es la versión final y robusta, consciente de la naturaleza no contigua de mmap.
 */
block_ptr coalesce_blocks(block_ptr block)
{
    // Fusionar con el bloque anterior si está libre y es físicamente contiguo.
    if (block->prev && block->prev->is_free &&
        ((char*)block->prev + BLOCK_META_SIZE + block->prev->size == (char*)block)) {
        log_event("coalesce: Fusing with prev %p", (void*)block->prev);
        block->prev->size += BLOCK_META_SIZE + block->size;
        block->prev->next = block->next;
        if (block->next) {
            block->next->prev = block->prev;
        }
        block = block->prev;
    }

    // Fusionar con el bloque siguiente si está libre y es físicamente contiguo.
    if (block->next && block->next->is_free &&
        ((char*)block + BLOCK_META_SIZE + block->size == (char*)block->next)) {
        log_event("coalesce: Fusing with next %p", (void*)block->next);
        block->size += BLOCK_META_SIZE + block->next->size;
        block->next = block->next->next;
        if (block->next) {
            block->next->prev = block;
        }
    }
    return block;
}


/**
 * @brief Obtiene el puntero al inicio del bloque de metadatos desde el puntero de usuario.
 */
block_ptr get_block_from_ptr(void* p)
{
    return (block_ptr)((char*)p - BLOCK_META_SIZE);
}

/**
 * @brief Valida si una dirección de puntero corresponde a un área de datos asignada por nuestro gestor.
 */
bool is_valid_address(void* p)
{
    if (!p || !heap_base) {
        return false;
    }
    block_ptr current = heap_base;
    while (current) {
        // La dirección es válida si apunta al inicio del área de datos de un bloque NO libre.
        if (!current->is_free && (void*)((char*)current + BLOCK_META_SIZE) == p) {
            return true;
        }
        current = current->next;
    }
    return false;
}

/**
 * @brief Reinicia el heap para testing. ¡PELIGROSO! Causa fugas de memoria.
 */
void reset_heap_for_testing(void)
{
    // Esta función es intencionadamente simple y peligrosa.
    // Simplemente olvida todos los bloques anteriores. `munmap` sería más limpio pero más complejo.
    heap_base = NULL;
    log_event("====== HEAP RESET FOR NEW TEST ======");
}