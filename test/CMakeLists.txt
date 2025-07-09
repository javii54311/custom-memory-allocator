# CMakeLists.txt para el directorio de Tests
#
# Este archivo define cómo se construye y se registra el ejecutable
# que contiene todas nuestras pruebas unitarias.

# --- Creación del Ejecutable de Pruebas ---

# 'add_executable()' crea un programa ejecutable.
#
# 'run_tests': Es el nombre del target y del archivo ejecutable que se generará.
#
# Lista de archivos fuente: Todos los archivos .c que se necesitan para construir
# el ejecutable. Esto incluye el punto de entrada (main_test_runner.c) y
# todas las suites de tests que hemos escrito.
add_executable(run_tests
    main_test_runner.c
    test_allocations.c
    test_policies.c
    test_heap_consistency.c
)


# --- Enlazado de Librerías ---

# 'target_link_libraries()' conecta nuestro ejecutable con las librerías que necesita.
#
# 'run_tests': El target al que le vamos a añadir las dependencias.
#
# 'PRIVATE': Significa que estas dependencias son necesarias para compilar 'run_tests',
#            pero no se propagan a otros targets que pudieran depender de 'run_tests'.
#
# 'memory': Nuestra librería de gestión de memoria. La enlazamos para poder llamar
#           a funciones como custom_malloc, set_allocation_policy, etc.
#
# 'unity': La librería de testing. La enlazamos para poder usar las macros de
#          Unity como RUN_TEST, TEST_ASSERT_EQUAL_INT, etc.
#
# Nota: CMake es lo suficientemente inteligente como para saber que 'memory' es una
# librería compartida (.so) y 'unity' es estática (.a), y las enlazará correctamente.
target_link_libraries(run_tests PRIVATE memory unity)


# --- Registro en CTest ---

# 'add_test()' registra un test en el framework CTest de CMake.
#
# 'NAME MemoryLibTests': Es el nombre que le damos a este test dentro de CTest.
#                        Cuando ejecutemos 'ctest', veremos este nombre.
#
# 'COMMAND run_tests': Es el comando que CTest debe ejecutar para correr este test.
#                      En este caso, simplemente ejecuta nuestro programa 'run_tests'.
add_test(NAME MemoryLibTests COMMAND run_tests)