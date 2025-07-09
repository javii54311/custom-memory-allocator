/**
 * @file allocator.c
 * @brief Implementación de la API de asignación de memoria (malloc, free, etc.).
 *
 * Contiene la implementación del "guardia de recursión" para evitar llamadas
 * cíclicas cuando las funciones de libc (como printf, opendir) internamente
 * llaman a malloc.
 */
#include "heap.h"
#include "log.h"
#include "memory_allocator.h"
#include <string.h>
#include <pthread.h> // Para almacenamiento local de hilo (thread-local storage)

// --- Guardia de Recursión ---
// Usamos '__thread' que es una extensión de GCC/Clang para crear una variable
// "thread-local". Cada hilo (thread) del programa tendrá su propia copia de
// esta variable. Esto hace que nuestro asignador sea seguro en entornos multihilo.
// Para un programa de un solo hilo, funciona como una variable estática normal.
static __thread bool is_inside_allocator = false;

/**
 * @brief Asigna un bloque de memoria del tamaño solicitado.
 */
void* custom_malloc(size_t size)
{
    // --- Comienzo del Guardia ---
    if (is_inside_allocator) {
        // Si ya estamos dentro, delegamos al malloc original para evitar recursión.
        // Esto es crucial para funciones como printf o dlsym que pueden llamar a malloc.
        return get_original_malloc(size);
    }
    is_inside_allocator = true; // Activamos la bandera para esta llamada.
    // --- Fin del Guardia ---

    if (size == 0) {
        is_inside_allocator = false; // Desactivar antes de salir.
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
            if (!block) {
                is_inside_allocator = false; // Desactivar antes de salir.
                return NULL;
            }
        }
    } else {
        block = extend_heap(NULL, aligned_size);
        if (!block) {
            is_inside_allocator = false; // Desactivar antes de salir.
            return NULL;
        }
        heap_base = block;
    }

    void* user_ptr = (char*)block + BLOCK_META_SIZE;
    log_event("malloc: Requested %zu, Allocated %zu at %p", size, aligned_size, user_ptr);

    is_inside_allocator = false; // Desactivamos la bandera justo antes de retornar.
    return user_ptr;
}

/**
 * @brief Libera un bloque de memoria previamente asignado.
 */
void custom_free(void* p)
{
    // --- Comienzo del Guardia ---
    if (is_inside_allocator) {
        // En este caso no hay un "free" original que llamar, simplemente no hacemos nada
        // para evitar cualquier efecto secundario no deseado durante una recursión.
        return;
    }
    is_inside_allocator = true;
    // --- Fin del Guardia ---

    if (!p) {
        is_inside_allocator = false;
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

    is_inside_allocator = false;
}

/**
 * @brief Asigna memoria para un array de elementos y la inicializa a cero.
 */
void* custom_calloc(size_t number, size_t size)
{
    size_t total_size = number * size;
    if (number != 0 && total_size / number != size) { // Comprobación de desbordamiento
        return NULL;
    }

    // La lógica de calloc simplemente llama a malloc, que ya está protegido por el guardia.
    void* p = custom_malloc(total_size);
    if (p) {
        // Usar total_size aquí es seguro porque malloc nos garantiza al menos ese tamaño.
        memset(p, 0, total_size);
        log_event("calloc: Allocated and zeroed %zu bytes at %p", total_size, p);
    }
    return p;
}

/**
 * @brief Cambia el tamaño de un bloque de memoria previamente asignado.
 */
void* custom_realloc(void* p, size_t size)
{
    // La lógica de realloc llama a malloc/free, que ya están protegidos.
    if (!p) {
        return custom_malloc(size);
    }
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

    // Intenta fusionar con el siguiente bloque si está libre y es suficiente.
    if (block->next && block->next->is_free && (block->size + BLOCK_META_SIZE + block->next->size) >= aligned_size) {
        coalesce_blocks(block); // La fusión ocurre aquí.
        split_block(block, aligned_size); // Se ajusta al nuevo tamaño.
        log_event("realloc: Realloc at %p by coalescing forward", p);
        return p;
    }

    // Si no se puede expandir in-situ, se mueve a una nueva ubicación.
    void* new_ptr = custom_malloc(aligned_size);
    if (!new_ptr) {
        return NULL;
    }

    memcpy(new_ptr, p, block->size); // Copia los datos antiguos.
    custom_free(p); // Libera el bloque original.

    log_event("realloc: Moved block from %p to %p (new size %zu)", p, new_ptr, aligned_size);
    return new_ptr;
}