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
    if (policy >= FIRST_FIT && policy <= WORST_FIT)
    {
        current_policy = policy;
    }
}

/**
 * @brief Recorre la lista enlazada del heap en busca de errores lógicos.
 */
void check_heap_consistency(void)
{
    block_ptr current = heap_base;
    while (current)
    {
        // Error 1: Puntero 'prev' del bloque siguiente no apunta al bloque actual.
        if (current->next && current->next->prev != current)
        {
            fprintf(stderr, "Heap Inconsistency: block %p's next->prev does not point back to it!\n", (void*)current);
        }
        // Error 2: Dos bloques libres son contiguos, indicando un fallo en la fusión.
        if (current->is_free && current->next && current->next->is_free)
        {
            fprintf(stderr, "Heap Inconsistency: adjacent free blocks %p and %p not coalesced!\n", (void*)current,
                    (void*)current->next);
        }
        current = current->next;
    }
}