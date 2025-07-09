/**
 * @file test_policies.c
 * @brief Suite de tests para evaluar y comparar las políticas de asignación de memoria.
 */
#include "memory_allocator.h"
#include "memory_control.h"
#include "memory_stats.h"
#include "unity.h"
#include <stdlib.h>
#include <time.h>

// Prototipo de la función runner, requerida por main_test_runner.c
void run_policy_tests(void);

// --- Constantes para la Simulación ---
#define NUM_ALLOCS 1000    // Número de asignaciones a realizar.
#define MAX_ALLOC_SIZE 256 // Tamaño máximo de cada asignación individual.

/**
 * @brief Función helper que ejecuta una carga de trabajo y mide el rendimiento.
 *
 * Simula un escenario de uso intensivo: muchas asignaciones de tamaños variables
 * seguidas de liberaciones parciales para inducir fragmentación. Mide el tiempo
 * y la tasa de fragmentación resultante para una política de asignación dada.
 *
 * @param policy_name Nombre de la política para imprimir en los resultados.
 */
void run_workload_and_measure(const char* policy_name)
{
    void* pointers[NUM_ALLOCS] = {NULL}; // Array para guardar los punteros asignados.
    clock_t start, end;
    double cpu_time_used;
    double fragmentation;

    // --- Fase de Asignación ---
    start = clock();
    for (int i = 0; i < NUM_ALLOCS; i++)
    {
        // Asignamos bloques de tamaños aleatorios entre 1 y MAX_ALLOC_SIZE.
        pointers[i] = custom_malloc(1 + (rand() % MAX_ALLOC_SIZE));
    }
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("\nPolicy: %s\n", policy_name);
    printf("  Allocation time: %f seconds\n", cpu_time_used);

    // --- Fase de Liberación Parcial ---
    // Liberamos la mitad de los punteros (los de índice par) para crear "agujeros".
    for (int i = 0; i < NUM_ALLOCS; i += 2)
    {
        custom_free(pointers[i]);
        pointers[i] = NULL;
    }

    // Medimos la fragmentación resultante después de la liberación parcial.
    fragmentation = get_fragmentation_rate();
    printf("  Fragmentation after partial free: %f\n", fragmentation);

    // --- Fase de Limpieza ---
    // Liberamos todos los punteros restantes para no afectar a los siguientes tests.
    for (int i = 0; i < NUM_ALLOCS; i++)
    {
        if (pointers[i] != NULL)
        {
            custom_free(pointers[i]);
        }
    }
}

/**
 * @brief Test principal que ejecuta la simulación para cada política de asignación.
 *
 * Este test no tiene aserciones de PASS/FAIL, su propósito es generar
 * datos de rendimiento que se puedan analizar en el informe del TP.
 */
void test_allocation_policies_performance(void)
{
    srand(time(NULL)); // Inicializamos la semilla para que los tamaños aleatorios varíen en cada ejecución.

    // --- Prueba con First Fit ---
    reset_heap_for_testing(); // Aseguramos un heap limpio antes de cada prueba.
    set_allocation_policy(FIRST_FIT);
    run_workload_and_measure("First Fit");

    // --- Prueba con Best Fit ---
    reset_heap_for_testing();
    set_allocation_policy(BEST_FIT);
    run_workload_and_measure("Best Fit");

    // --- Prueba con Worst Fit ---
    reset_heap_for_testing();
    set_allocation_policy(WORST_FIT);
    run_workload_and_measure("Worst Fit");

    TEST_PASS(); // Marcamos el test como pasado si se completó sin crashes.
}

/**
 * @brief Agrupa y ejecuta todos los tests de políticas.
 */
void run_policy_tests(void)
{
    RUN_TEST(test_allocation_policies_performance);
}