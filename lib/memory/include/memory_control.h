/**
 * @file memory_control.h
 * @brief API pública para el control, configuración y depuración del asignador de memoria.
 */

#ifndef MEMORY_CONTROL_H
#define MEMORY_CONTROL_H

/**
 * @brief Define las políticas de asignación de memoria disponibles para encontrar bloques libres.
 */
typedef enum
{
    FIRST_FIT = 0, ///< Usa el primer bloque libre que sea suficientemente grande.
    BEST_FIT = 1,  ///< Usa el bloque libre más pequeño que sea suficientemente grande.
    WORST_FIT = 2  ///< Usa el bloque libre más grande disponible.
} allocation_policy_t;

/**
 * @brief Configura la política de asignación de memoria que `custom_malloc` utilizará.
 *
 * @param policy La política a utilizar (FIRST_FIT, BEST_FIT, o WORST_FIT).
 */
void set_allocation_policy(allocation_policy_t policy);

/**
 * @brief Realiza una verificación de la integridad estructural del heap.
 *
 * Recorre la lista de bloques de memoria para detectar inconsistencias comunes como:
 * - Enlaces `prev`/`next` corruptos.
 * - Bloques libres adyacentes que deberían haber sido fusionados.
 * Imprime los errores encontrados en stderr para facilitar la depuración.
 */
void check_heap_consistency(void);

/**
 * @brief Inicializa el sistema de registro de eventos de memoria.
 *
 * Abre un archivo en modo escritura donde se registrarán todas las operaciones
 * (malloc, free, etc.) para su posterior análisis.
 *
 * @param filename Ruta al archivo de log. Si es NULL, el logging se deshabilita.
 */
void init_memory_log(const char* filename);

/**
 * @brief Cierra el archivo de log si estaba abierto, liberando el descriptor de archivo.
 */
void close_memory_log(void);

/**
 * @brief Reinicia el estado del heap a su estado inicial (vacío).
 *
 * Esta función está diseñada exclusivamente para testing, para asegurar que
 * cada prueba se ejecute en un entorno aislado y predecible.
 *
 * @warning ¡SOLO PARA TESTING! Esta función abandona la memoria previamente
 * asignada, causando fugas de memoria (memory leaks). No usar en producción.
 */
void reset_heap_for_testing(void);

#endif // MEMORY_CONTROL_H