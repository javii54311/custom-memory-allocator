/**
 * @file heap.h
 * @brief Declaraciones internas para la gestión del heap.
 *
 * Este es un header PRIVADO. No debe ser incluido por clientes externos.
 * Define la estructura de los bloques de memoria, constantes críticas
 * y los prototipos de las funciones de bajo nivel que manipulan directamente
 * la lista de bloques del heap.
 */
#ifndef HEAP_H
#define HEAP_H

#include "memory_control.h" // Para allocation_policy_t
#include <stdbool.h>
#include <stddef.h>

// --- Constantes del Heap ---

/** @brief Alineación de memoria en bytes. Todas las asignaciones se alinearán a este valor. */
#define ALIGNMENT 8

/**
 * @brief Macro para alinear un tamaño de bytes al siguiente múltiplo de ALIGNMENT.
 *
 * Esta operación matemática asegura que todos los bloques de memoria y sus
 * metadatos comiencen en direcciones de memoria que son múltiplos de 8,
 * lo cual es eficiente y a menudo requerido por ciertas arquitecturas de CPU.
 */
#define ALIGN(size) (((size - 1) / ALIGNMENT) * ALIGNMENT + ALIGNMENT)

/**
 * @brief Tamaño de los metadatos de un bloque, alineado.
 *
 * Calcula el espacio que ocupa la estructura de control (s_block) y lo alinea
 * para asegurar que el área de datos subsiguiente también comience en una
 * dirección de memoria alineada.
 */
#define BLOCK_META_SIZE ALIGN(sizeof(struct s_block))

// --- Estructura del Bloque de Memoria ---

/**
 * @struct s_block
 * @brief Estructura de metadatos que precede a cada bloque de datos en el heap.
 *
 * Esta estructura forma una lista doblemente enlazada de todos los bloques
 * de memoria, tanto libres como ocupados.
 */
struct s_block
{
    size_t size;          ///< Tamaño del área de datos (no incluye estos metadatos).
    struct s_block* next; ///< Puntero al siguiente bloque en la lista.
    struct s_block* prev; ///< Puntero al bloque anterior en la lista.
    bool is_free;         ///< Flag booleano: true si el bloque está libre, false si está ocupado.
    void* data_ptr;       ///< Puntero de "verificación" al inicio del área de datos. Ayuda a validar punteros.
    char data[1];         ///< Marcador flexible. El área de datos del usuario comienza aquí.
};
typedef struct s_block* block_ptr;

// --- Variables Globales del Módulo ---

/** @brief Puntero a la base (inicio) de la lista de bloques del heap. */
extern block_ptr heap_base;

/** @brief Política de asignación actualmente en uso (FIRST_FIT, BEST_FIT, WORST_FIT). */
extern allocation_policy_t current_policy;

// --- Prototipos de Funciones Internas ---

block_ptr find_free_block(block_ptr* last, size_t size);
block_ptr extend_heap(block_ptr last, size_t size);
void split_block(block_ptr block, size_t size);
block_ptr coalesce_blocks(block_ptr block);
block_ptr get_block_from_ptr(void* p);
bool is_valid_address(void* p);

#endif // HEAP_H