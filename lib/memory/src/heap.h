/**
 * @file heap.h
 * @brief Declaraciones internas para la gestión del heap.
 */
#ifndef HEAP_H
#define HEAP_H

#include "memory_control.h" // Para allocation_policy_t
#include <stdbool.h>
#include <stddef.h>

// --- Constantes del Heap ---

/** @brief Alineación de memoria en bytes. */
#define ALIGNMENT 8

/**
 * @brief Macro para alinear un tamaño al siguiente múltiplo de ALIGNMENT.
 * Esta es una forma estándar y eficiente de hacerlo usando operaciones de bits.
 */
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

// --- Estructura del Bloque de Memoria ---

/**
 * @struct s_block
 * @brief Estructura de metadatos que precede a cada bloque de datos en el heap.
 */
struct s_block
{
    size_t size;          ///< Tamaño del área de datos del usuario (no incluye estos metadatos).
    struct s_block* next; ///< Puntero al siguiente bloque en la lista.
    struct s_block* prev; ///< Puntero al bloque anterior en la lista.
    bool is_free;         ///< Flag booleano: true si el bloque está libre, false si está ocupado.
    void* data_ptr;       ///< Puntero de "verificación" al inicio del área de datos. Útil para depurar.
    char data[];          ///< [CORRECCIÓN CLAVE] Miembro de array flexible (C99). El área de datos del usuario comienza aquí.
};
typedef struct s_block* block_ptr;

/**
 * @brief Tamaño de los metadatos de un bloque.
 *
 * [CORRECCIÓN CLAVE] Se calcula usando offsetof, que da el tamaño de la estructura
 * sin incluir el flexible array member. Es la forma más robusta y estándar.
 */
#define BLOCK_META_SIZE ALIGN(offsetof(struct s_block, data))


// --- Variables Globales del Módulo ---
extern block_ptr heap_base;
extern allocation_policy_t current_policy;

// --- Prototipos de Funciones Internas ---
block_ptr find_free_block(block_ptr* last, size_t size);
block_ptr extend_heap(block_ptr last, size_t size);
void split_block(block_ptr block, size_t size);
block_ptr coalesce_blocks(block_ptr block);
block_ptr get_block_from_ptr(void* p);
bool is_valid_address(void* p);

#endif // HEAP_H