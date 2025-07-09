/**
 * @file memory_stats.h
 * @brief API pública para obtener estadísticas y métricas del estado actual del heap.
 */

#ifndef MEMORY_STATS_H
#define MEMORY_STATS_H

#include <stddef.h> // Para size_t

/**
 * @brief Reporta el uso detallado de la memoria gestionada por el asignador.
 *
 * Recorre el heap y calcula el total de memoria en uso y libre, así como el
 * número de bloques en cada estado.
 *
 * @param[out] total_allocated Puntero donde se almacenará el total de bytes asignados.
 * @param[out] total_free Puntero donde se almacenará el total de bytes en bloques libres.
 * @param[out] allocated_blocks Puntero donde se almacenará el número de bloques asignados.
 * @param[out] free_blocks Puntero donde se almacenará el número de bloques libres.
 */
void memory_usage_stats(size_t* total_allocated, size_t* total_free, int* allocated_blocks, int* free_blocks);

/**
 * @brief Calcula la tasa de fragmentación externa del heap.
 *
 * La fragmentación externa ocurre cuando hay suficiente memoria libre total,
 * pero está dividida en muchos bloques pequeños no contiguos.
 * Se define como: 1.0 - (tamaño del bloque libre más grande / memoria libre total).
 *
 * @return La tasa de fragmentación como un valor entre 0.0 (ideal) y 1.0 (muy fragmentado).
 *         Devuelve 0.0 si no hay memoria libre.
 */
double get_fragmentation_rate(void);

#endif // MEMORY_STATS_H