/**
 * @file heap.h
 * @brief Declaraciones internas para la gestión del heap.
 */
#ifndef HEAP_H
#define HEAP_H

#include "memory_control.h"
#include <stdbool.h>
#include <stddef.h>

// --- Constantes del Heap ---
#define ALIGNMENT 8
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

// --- Estructura del Bloque de Memoria (Simplificada) ---
/**
 * @struct s_block
 * @brief Estructura de metadatos que precede a cada bloque de datos en el heap.
 * [CORRECCIÓN FINAL] Se elimina el data_ptr redundante y el flexible array member
 * para una definición más simple y portable. El área de datos del usuario
 * comienza inmediatamente después de esta estructura.
 */
struct s_block
{
    size_t size;          ///< Tamaño del área de datos del usuario (no incluye metadatos).
    struct s_block* next; ///< Puntero al siguiente bloque en la lista.
    struct s_block* prev; ///< Puntero al bloque anterior en la lista.
    bool is_free;         ///< Flag booleano: true si el bloque está libre, false si está ocupado.
};
typedef struct s_block* block_ptr;

/**
 * @brief Tamaño de los metadatos de un bloque.
 * [CORRECCIÓN FINAL] Se calcula directamente con sizeof sobre la estructura simplificada.
 */
#define BLOCK_META_SIZE ALIGN(sizeof(struct s_block))

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