/**
 * @file control.c
 * @brief Implementación de la API de control y depuración del asignador.
 */
#include "heap.h"
#include "memory_control.h"
#include <stdio.h>

/**
 * @brief Establece la política global para la búsqueda de bloques libres.
 */
void set_allocation_policy(allocation_policy_t policy)
{
    // Se valida que la política esté dentro del rango definido en el enum.
    if (policy >= FIRST_FIT && policy <= WORST_FIT) {
        current_policy = policy;
    }
}

/**
 * @brief Recorre la lista enlazada del heap en busca de errores lógicos.
 *
 * Esta versión corregida es consciente de que los bloques pueden no ser contiguos
 * si fueron asignados por diferentes llamadas a mmap, por lo que solo reporta
 * una inconsistencia de fusión si dos bloques libres son adyacentes Y contiguos.
 */
void check_heap_consistency(void)
{
    block_ptr current = heap_base;
    while (current) {
        // Error 1: Puntero 'prev' del bloque siguiente no apunta al bloque actual.
        if (current->next && current->next->prev != current) {
            fprintf(stderr, "Heap Inconsistency: block %p's next->prev does not point back to it!\n", (void*)current);
        }

        // Error 2: Dos bloques son libres y adyacentes en la lista.
        // [LA CORRECCIÓN] Ahora también comprobamos si son físicamente contiguos.
        // Solo si ambas condiciones se cumplen, es un error de fusión.
        if (current->is_free && current->next && current->next->is_free) {
            // Comprobamos la contigüidad física. El final del bloque actual debe ser el inicio del siguiente.
            bool are_contiguous = ((char*)current + BLOCK_META_SIZE + current->size) == (char*)current->next;
            if (are_contiguous) {
                fprintf(stderr, "Heap Inconsistency: adjacent and contiguous free blocks %p and %p not coalesced!\n",
                        (void*)current, (void*)current->next);
            }
        }
        current = current->next;
    }
}

// Nota: reset_heap_for_testing se movió a heap.c ya que opera directamente
// sobre la variable global heap_base. Las demás funciones de control están aquí.