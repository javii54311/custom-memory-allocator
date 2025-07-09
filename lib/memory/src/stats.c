/**
 * @file stats.c
 * @brief Implementación de la API de estadísticas y métricas del heap.
 *
 * Este archivo proporciona funciones para consultar el estado del heap,
 * como el uso de memoria y la tasa de fragmentación, recorriendo la lista de bloques.
 */
#include "heap.h"
#include "memory_stats.h"

/**
 * @brief Calcula y reporta el uso detallado de la memoria del heap.
 *
 * Itera a través de toda la lista de bloques y clasifica cada uno como
 * 'asignado' o 'libre', sumando los tamaños y contando los bloques en cada categoría.
 * Los resultados se devuelven a través de los punteros proporcionados como argumentos.
 */
void memory_usage_stats(size_t* total_allocated, size_t* total_free, int* allocated_blocks, int* free_blocks)
{
    // Inicializamos todos los contadores a cero antes de empezar.
    *total_allocated = 0;
    *total_free = 0;
    *allocated_blocks = 0;
    *free_blocks = 0;

    block_ptr current = heap_base;
    while (current)
    {
        if (current->is_free)
        {
            *total_free += current->size;
            (*free_blocks)++;
        }
        else
        {
            *total_allocated += current->size;
            (*allocated_blocks)++;
        }
        current = current->next;
    }
}

/**
 * @brief Calcula la tasa de fragmentación externa actual del heap.
 *
 * La fragmentación es un indicador clave de la eficiencia de un asignador de memoria.
 * Una alta fragmentación significa que hay mucha memoria libre pero en trozos pequeños,
 * lo que puede impedir asignaciones grandes.
 *
 * @return Un valor de 0.0 a 1.0, donde 0.0 es ideal (sin fragmentación) y 1.0 es lo peor.
 */
double get_fragmentation_rate(void)
{
    size_t total_free_mem = 0;
    size_t largest_free_block = 0;

    // Recorremos la lista una vez para encontrar dos valores:
    // 1. La suma total de toda la memoria libre.
    // 2. El tamaño del bloque libre más grande que podríamos asignar.
    block_ptr current = heap_base;
    while (current)
    {
        if (current->is_free)
        {
            total_free_mem += current->size;
            if (current->size > largest_free_block)
            {
                largest_free_block = current->size;
            }
        }
        current = current->next;
    }

    // Si no hay memoria libre, no puede haber fragmentación.
    if (total_free_mem == 0)
    {
        return 0.0;
    }

    // La fórmula mide qué proporción de la memoria libre "se desperdicia"
    // al no ser parte del bloque contiguo más grande.
    return 1.0 - ((double)largest_free_block / (double)total_free_mem);
}