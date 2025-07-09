/**
 * @file test_allocations.c
 * @brief Suite de tests para las funciones básicas de asignación (malloc, free, calloc).
 */
#include "memory_allocator.h" // Incluimos la API pública que vamos a probar
#include "unity.h"

// Prototipo de la función runner, requerida por main_test_runner.c
void run_allocation_tests(void);

// --- Casos de Prueba Individuales ---

/**
 * @brief Verifica el caso más simple: que custom_malloc devuelva un puntero no nulo.
 */
void test_malloc_should_return_non_null_pointer(void)
{
    void* ptr = custom_malloc(128);
    TEST_ASSERT_NOT_NULL(ptr);
    custom_free(ptr); // Liberamos para no dejar basura y probar free de paso.
}

/**
 * @brief Verifica que llamar a custom_free con un puntero válido no cause un crash.
 *
 * Este es un test de "humo" (smoke test). Si esto falla, algo muy fundamental está roto.
 */
void test_free_a_valid_pointer_should_not_crash(void)
{
    void* ptr = custom_malloc(10);
    // La prueba real es que la siguiente línea no cause un segmentation fault.
    custom_free(ptr);
    TEST_PASS(); // Si llegamos aquí, la prueba pasa.
}

/**
 * @brief Verifica que la memoria asignada por custom_calloc esté correctamente inicializada a cero.
 */
void test_calloc_memory_is_zeroed(void)
{
    size_t count = 100;
    // Pedimos memoria para 100 bytes.
    unsigned char* mem = custom_calloc(count, sizeof(unsigned char));
    TEST_ASSERT_NOT_NULL(mem);

    // Verificamos que cada byte en la región asignada sea cero.
    for (size_t i = 0; i < count; i++)
    {
        if (mem[i] != 0)
        {
            // Si encontramos un byte no nulo, fallamos el test inmediatamente con un mensaje claro.
            TEST_FAIL_MESSAGE("Memory not zeroed by calloc!");
        }
    }

    // Si el bucle termina sin fallar, significa que toda la memoria era cero.
    TEST_PASS();

    // Limpieza final.
    custom_free(mem);
}

// --- Función Runner de la Suite de Tests ---

/**
 * @brief Agrupa y ejecuta todos los tests definidos en este archivo.
 */
void run_allocation_tests(void)
{
    RUN_TEST(test_malloc_should_return_non_null_pointer);
    RUN_TEST(test_free_a_valid_pointer_should_not_crash);
    RUN_TEST(test_calloc_memory_is_zeroed);
}