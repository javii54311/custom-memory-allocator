/**
 * @file main_test_runner.c
 * @brief Punto de entrada principal para la suite de tests de la librería de memoria.
 *
 * Este archivo utiliza el framework Unity para orquestar la ejecución de todas
 * las pruebas unitarias. Define las funciones globales setUp y tearDown, que
 * se ejecutan antes y después de cada test individual, y luego llama a las
 * funciones que agrupan los tests de cada módulo.
 */
#include "memory_control.h" // Necesario para reset_heap_for_testing e init/close_log
#include "unity.h"

// --- Declaración de las Suites de Tests ---
// Prototipos de las funciones 'runner' de cada archivo de test.
// Cada una de estas funciones es responsable de ejecutar todos los tests
// definidos en su respectivo archivo .c.
void run_allocation_tests(void);
void run_consistency_tests(void);
void run_policy_tests(void);

// --- Funciones de Configuración y Limpieza de Unity ---

/**
 * @brief Función de configuración ejecutada por Unity ANTES de cada test.
 *
 * Se utiliza para asegurar un entorno limpio y predecible para cada prueba.
 * Aquí, reseteamos el estado del heap y reiniciamos el archivo de log.
 */
void setUp(void)
{
    // Inicializa el log para que cada test tenga un archivo limpio donde escribir.
    init_memory_log("test_run.log");
    // ¡CRUCIAL! Resetea el heap para evitar que los tests interfieran entre sí.
    reset_heap_for_testing();
}

/**
 * @brief Función de limpieza ejecutada por Unity DESPUÉS de cada test.
 *
 * Libera los recursos utilizados durante la prueba.
 */
void tearDown(void)
{
    // Cierra el descriptor de archivo del log para ser prolijos.
    close_memory_log();
}

// --- Punto de Entrada Principal de los Tests ---

/**
 * @brief La función principal del ejecutable de tests.
 *
 * Inicializa Unity, ejecuta todas las suites de tests en orden,
 * y finalmente presenta el resumen de los resultados.
 */
int main(void)
{
    // Inicia el entorno de Unity.
    UNITY_BEGIN();

    // Ejecuta cada una de las suites de tests.
    // La macro RUN_TEST de Unity se encarga de llamar a setUp/tearDown
    // alrededor de cada test individual dentro de estas funciones.
    RUN_TEST(run_allocation_tests);
    RUN_TEST(run_consistency_tests);
    RUN_TEST(run_policy_tests);

    // Finaliza el entorno de Unity y devuelve el código de estado (0 si todo OK).
    return UNITY_END();
}