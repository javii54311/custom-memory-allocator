/**
 * @file memory_allocator.h
 * @brief API pública para las funciones de asignación de memoria estándar.
 *
 * Este archivo define los prototipos para las funciones de gestión de memoria
 * que sobreescriben las de la libc: malloc, free, calloc y realloc.
 * Es la interfaz principal para cualquier programa que utilice esta librería.
 */

#ifndef MEMORY_ALLOCATOR_H
#define MEMORY_ALLOCATOR_H

#include <stddef.h> // Para size_t

/**
 * @brief Asigna un bloque de memoria del tamaño solicitado.
 *
 * Utiliza la política de asignación actualmente configurada para encontrar
 * y asignar un bloque de memoria contiguo.
 *
 * @param size Tamaño en bytes del bloque a asignar.
 * @return void* Puntero al inicio del área de datos asignada, o NULL si falla la asignación.
 */
void* custom_malloc(size_t size);

/**
 * @brief Libera un bloque de memoria previamente asignado.
 *
 * Marca el bloque como libre y lo fusiona con bloques adyacentes si también
 * están libres para reducir la fragmentación (coalescing).
 *
 * @param p Puntero al área de datos a liberar. Si es NULL, la función no hace nada.
 */
void custom_free(void* p);

/**
 * @brief Asigna memoria para un array de elementos y la inicializa a cero.
 *
 * @param number Número de elementos a asignar.
 * @param size Tamaño en bytes de cada elemento.
 * @return void* Puntero al inicio del área de datos asignada e inicializada, o NULL si falla.
 */
void* custom_calloc(size_t number, size_t size);

/**
 * @brief Cambia el tamaño de un bloque de memoria previamente asignado.
 *
 * Intenta redimensionar el bloque en el sitio si es posible. Si no,
 * asigna un nuevo bloque, copia los datos antiguos y libera el original.
 *
 * @param p Puntero al área de datos a redimensionar.
 * @param size Nuevo tamaño en bytes del bloque.
 * @return void* Puntero al área de datos redimensionada, o NULL si falla.
 */
void* custom_realloc(void* p, size_t size);

#endif // MEMORY_ALLOCATOR_H