/**
 * @file heap.h
 * @brief Declaraciones internas para la gestión del heap.
 */
#ifndef HEAP_H
#define HEAP_H

#include "memory_control.h"
#include <stdbool.h>
#include <stddef.h>
#include <dlfcn.h> // Necesario para dlsym

// --- Constantes del Heap ---
#define ALIGNMENT 8
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

// --- Estructura del Bloque de Memoria ---
struct s_block
{
    size_t size;
    struct s_block* next;
    struct s_block* prev;
    bool is_free;
};
typedef struct s_block* block_ptr;

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
void* get_original_malloc(size_t size); // Prototipo para llamar al malloc de libc

#endif // HEAP_H