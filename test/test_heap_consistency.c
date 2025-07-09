/**
 * @file test_heap_consistency.c
 * @brief Suite de tests para las operaciones estructurales del heap (split, coalesce, realloc).
 *
 * Estas pruebas verifican que la lista de bloques se mantenga consistente
 * después de operaciones de división, fusión y redimensionamiento.
 */
#include "memory_allocator.h"
#include "memory_control.h"
#include "memory_stats.h"
#include "unity.h"
#include <string.h>

// Prototipo de la función runner, requerida por main_test_runner.c
void run_consistency_tests(void);

// --- Casos de Prueba Individuales ---

/**
 * @brief Verifica que la lógica de fusión (coalesce) funcione correctamente en cascada.
 */
void test_coalescing_logic_is_correct(void)
{
    // Gracias a setUp() y reset_heap_for_testing(), el heap está vacío al empezar.
    int allocated_blocks, initial_free_blocks, free_blocks_after_op;
    size_t total_alloc, total_free;
    memory_usage_stats(&total_alloc, &total_free, &allocated_blocks, &initial_free_blocks);
    TEST_ASSERT_EQUAL_INT(0, initial_free_blocks);

    // 1. Asignamos 3 bloques para crear el escenario: [p1][p2][p3]
    void* p1 = custom_malloc(100);
    void* p2 = custom_malloc(100);
    void* p3 = custom_malloc(100);
    TEST_ASSERT_NOT_NULL(p1);
    TEST_ASSERT_NOT_NULL(p2);
    TEST_ASSERT_NOT_NULL(p3);

    // 2. Liberamos el del medio. No hay fusión. El número de bloques libres debe aumentar en 1.
    custom_free(p2);
    memory_usage_stats(&total_alloc, &total_free, &allocated_blocks, &free_blocks_after_op);
    TEST_ASSERT_EQUAL_INT(initial_free_blocks + 1, free_blocks_after_op);

    // 3. Liberamos el primero. Debe fusionarse hacia adelante con p2. El número total de bloques libres no cambia.
    custom_free(p1);
    memory_usage_stats(&total_alloc, &total_free, &allocated_blocks, &free_blocks_after_op);
    TEST_ASSERT_EQUAL_INT(initial_free_blocks + 1, free_blocks_after_op);

    // 4. Liberamos el último. Debe fusionarse hacia atrás con el bloque [p1+p2].
    // Al final, todos los bloques se han fusionado en uno solo.
    custom_free(p3);
    memory_usage_stats(&total_alloc, &total_free, &allocated_blocks, &free_blocks_after_op);
    TEST_ASSERT_EQUAL_INT(1, free_blocks_after_op);
}

/**
 * @brief Verifica que split_block divida correctamente un bloque libre grande cuando se solicita un trozo.
 */
void test_split_block_divides_a_large_free_block(void)
{
    // 1. Creamos un bloque grande y lo liberamos para tener un gran espacio libre.
    void* p_large = custom_malloc(2048);
    custom_free(p_large);

    int allocated_blocks, free_blocks;
    size_t total_allocated, total_free;
    memory_usage_stats(&total_allocated, &total_free, &allocated_blocks, &free_blocks);
    TEST_ASSERT_EQUAL_INT(1, free_blocks); // Debemos tener 1 solo gran bloque libre.

    // 2. Pedimos un trozo pequeño de ese espacio. Esto debe forzar una división.
    void* p_small = custom_malloc(128);
    TEST_ASSERT_NOT_NULL(p_small);

    // 3. Verificamos el resultado: el heap ahora debe tener 1 bloque ocupado (p_small)
    // y 1 bloque libre (el resto del bloque grande).
    memory_usage_stats(&total_allocated, &total_free, &allocated_blocks, &free_blocks);
    TEST_ASSERT_EQUAL_INT(1, allocated_blocks);
    TEST_ASSERT_EQUAL_INT(1, free_blocks);

    custom_free(p_small);
}

/**
 * @brief Verifica los casos básicos de realloc: achicar y agrandar (con movimiento).
 */
void test_realloc_logic(void)
{
    // Escenario 1: Achicar un bloque.
    char* str = custom_malloc(50);
    strcpy(str, "Este es un texto de prueba largo");
    char* new_str = custom_realloc(str, 20); // El nuevo tamaño es más pequeño.
    TEST_ASSERT_EQUAL_PTR(str, new_str);     // El puntero no debería haber cambiado.
    TEST_ASSERT_EQUAL_STRING("Este es un texto de prueba largo", new_str);

    // Escenario 2: Agrandar un bloque, forzando un movimiento.
    custom_malloc(16);                              // Asignamos un "tapón" para evitar la expansión in-situ.
    char* final_str = custom_realloc(new_str, 100); // Pedimos más memoria.
    TEST_ASSERT_NOT_NULL(final_str);
    TEST_ASSERT_NOT_EQUAL(new_str, final_str); // El puntero DEBERÍA haber cambiado.
    TEST_ASSERT_EQUAL_STRING("Este es un texto de prueba largo", final_str);

    custom_free(final_str);
}

/**
 * @brief Verifica el caso de realloc donde se expande un bloque in-situ.
 */
void test_realloc_expansion_in_place(void)
{
    // 1. Creamos dos bloques contiguos.
    char* p1 = custom_malloc(32);
    void* p2 = custom_malloc(32);
    strcpy(p1, "data");

    // 2. Liberamos el segundo bloque, creando espacio libre justo al lado del primero.
    custom_free(p2);

    // 3. Hacemos realloc en p1 pidiendo más espacio. Debería usar el espacio de p2.
    // El puntero NO debería cambiar, ya que la expansión es in-situ.
    char* p1_expanded = custom_realloc(p1, 64);
    TEST_ASSERT_EQUAL_PTR(p1, p1_expanded);
    TEST_ASSERT_EQUAL_STRING("data", p1_expanded); // Los datos deben preservarse.

    custom_free(p1_expanded);
}

/**
 * @brief Agrupa y ejecuta todos los tests de consistencia del heap.
 */
void run_consistency_tests(void)
{
    RUN_TEST(test_coalescing_logic_is_correct);
    RUN_TEST(test_split_block_divides_a_large_free_block);
    RUN_TEST(test_realloc_logic);
    RUN_TEST(test_realloc_expansion_in_place);
}