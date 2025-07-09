/**
 * @file allocator.c
 * @brief Implementación de la API de asignación de memoria (malloc, free, etc.).
 *
 * Este archivo contiene la lógica de alto nivel que un cliente utiliza.
 * Delega las operaciones complejas sobre el heap (como encontrar o extender bloques)
 * al módulo 'heap.c'.
 */
#include "heap.h"
#include "log.h"
#include "memory_allocator.h"
#include <string.h> // para memset y memcpy

void* custom_malloc(size_t size)
{
    if (size == 0)
    {
        return NULL;
    }

    size_t aligned_size = ALIGN(size);
    block_ptr block;
    block_ptr last = heap_base;

    if (heap_base)
    {
        // El heap ya existe, buscamos un bloque libre.
        block = find_free_block(&last, aligned_size);
        if (block)
        {
            // Se encontró un bloque libre adecuado.
            // Lo dividimos si es más grande de lo necesario.
            split_block(block, aligned_size);
            block->is_free = false;
        }
        else
        {
            // No se encontró un bloque libre, extendemos el heap.
            block = extend_heap(last, aligned_size);
            if (!block)
                return NULL;
        }
    }
    else
    {
        // El heap no ha sido inicializado, este es el primer malloc.
        // Extendemos el heap para crear el primer bloque.
        block = extend_heap(NULL, aligned_size);
        if (!block)
            return NULL;
        heap_base = block;
    }

    // [CORRECCIÓN CLAVE] El puntero de usuario es la dirección del bloque de metadatos
    // MÁS el tamaño de los metadatos. El área de datos comienza justo después.
    void* user_ptr = (char*)block + BLOCK_META_SIZE;

    // Actualizamos el data_ptr de verificación para que apunte al inicio del área de usuario.
    block->data_ptr = user_ptr;

    log_event("malloc: Requested %zu, Allocated %zu at %p", size, aligned_size, user_ptr);
    return user_ptr;
}

void custom_free(void* p)
{
    if (!p)
    {
        return; // free(NULL) no hace nada.
    }

    // Verificamos si la dirección es válida antes de intentar operarla.
    if (is_valid_address(p))
    {
        block_ptr block = get_block_from_ptr(p);
        block->is_free = true;
        log_event("free: Freeing memory at %p (size %zu)", p, block->size);
        // Intentamos fusionar el bloque recién liberado con sus vecinos.
        coalesce_blocks(block);
    }
    else
    {
        // Liberar un puntero inválido es un error grave. Lo registramos.
        log_event("free: Invalid pointer %p", p);
    }
}

void* custom_calloc(size_t number, size_t size)
{
    size_t total_size = number * size;
    // Chequeo de overflow (aunque poco probable con size_t)
    if (number != 0 && total_size / number != size)
    {
        return NULL;
    }
    if (total_size == 0)
    {
        return NULL;
    }

    // Usamos nuestro malloc para asignar la memoria.
    void* p = custom_malloc(total_size);
    if (p)
    {
        // Si la asignación fue exitosa, inicializamos la memoria a cero.
        // Ojo: el tamaño real asignado puede ser mayor por la alineación.
        // Usamos el tamaño del bloque para memset para ser precisos.
        block_ptr block = get_block_from_ptr(p);
        memset(p, 0, block->size);
        log_event("calloc: Allocated and zeroed %zu bytes at %p", block->size, p);
    }
    return p;
}

void* custom_realloc(void* p, size_t size)
{
    // Comportamiento estándar de realloc.
    if (!p)
        return custom_malloc(size);
    if (size == 0)
    {
        custom_free(p);
        return NULL;
    }

    if (!is_valid_address(p))
    {
        log_event("realloc: Invalid pointer %p", p);
        return NULL;
    }

    block_ptr block = get_block_from_ptr(p);
    size_t aligned_size = ALIGN(size);

    // Caso 1: El nuevo tamaño es más pequeño o igual.
    // Simplemente achicamos el bloque actual (si es posible) y devolvemos el mismo puntero.
    if (block->size >= aligned_size)
    {
        split_block(block, aligned_size);
        log_event("realloc: Shrunk block at %p to %zu bytes", p, aligned_size);
        return p;
    }

    // Caso 2: Expansión in-situ.
    // Si el bloque siguiente está libre y el espacio combinado es suficiente,
    // nos fusionamos con él para crecer sin mover los datos.
    if (block->next && block->next->is_free && (block->size + BLOCK_META_SIZE + block->next->size) >= aligned_size)
    {
        coalesce_blocks(block);           // El bloque actual se expandirá.
        split_block(block, aligned_size); // Lo ajustamos al nuevo tamaño.
        log_event("realloc: Realloc at %p by coalescing forward", p);
        return p;
    }

    // Caso 3: No hay más opción que mover.
    // Asignamos un nuevo bloque en otra parte, copiamos los datos y liberamos el antiguo.
    void* new_ptr = custom_malloc(aligned_size);
    if (!new_ptr)
        return NULL; // Falló la nueva asignación.

    memcpy(new_ptr, p, block->size); // Copiamos los datos antiguos.
    custom_free(p);                  // Liberamos el bloque original.

    log_event("realloc: Moved block from %p to %p (new size %zu)", p, new_ptr, aligned_size);
    return new_ptr;
}