# ==============================================================================
#                 CMakeLists.txt Principal - MemoryManager Project
# ==============================================================================
#
# Este archivo es el punto de entrada para la configuración de la compilación
# de todo el proyecto, incluyendo la librería de memoria, las dependencias
# externas y la suite de tests.

# --- Configuración del Proyecto ---

# Especifica la versión mínima de CMake requerida para procesar este archivo.
cmake_minimum_required(VERSION 3.14)

# Define el nombre del proyecto y el lenguaje principal (C).
# El nombre del proyecto se almacena en la variable ${PROJECT_NAME}.
project(MemoryManager C)

# Añade la bandera de compilación '-g' a TODOS los targets.
# Esto incluye símbolos de depuración en los binarios, lo cual es
# indispensable para poder usar herramientas como GDB de manera efectiva.
add_compile_options(-g)

# Establece el estándar de C a utilizar (C11).
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON) # Exige que el compilador soporte C11.


# --- Configuración Opcional para Cobertura de Código ---

# `option()` crea una variable de caché que se puede configurar desde la línea
# de comandos (ej. cmake -DENABLE_COVERAGE=ON ..).
# Por defecto, la cobertura está desactivada (OFF).
option(ENABLE_COVERAGE "Enable code coverage instrumentation" OFF)

# Si la opción de cobertura está activada...
if(ENABLE_COVERAGE)
    # Imprime un mensaje en la consola para confirmar que la opción fue reconocida.
    message(STATUS "Code coverage enabled. Compiling with --coverage flag.")
    
    # Añade la bandera '--coverage' a las opciones de compilación globales.
    # Esto le dice a GCC/Clang que instrumente el código para medir la cobertura,
    # generando los archivos .gcno durante la compilación.
    add_compile_options(--coverage)
    
    # Añade la bandera '--coverage' a las opciones de enlazado globales.
    # Esto asegura que los ejecutables se enlacen con la librería gcov,
    # necesaria para que el programa pueda escribir los archivos de datos .gcda
    # en tiempo de ejecución.
    add_link_options(--coverage)
endif()


# --- Gestión de Subdirectorios ---

# 'add_subdirectory()' le dice a CMake que entre en un directorio,
# busque otro archivo CMakeLists.txt allí, y lo procese.

# 1. Dependencias Externas:
#    Procesamos primero las dependencias para que sus targets (como 'unity')
#    estén definidos y disponibles para el resto del proyecto.
add_subdirectory(external/unity)

# 2. Componentes del Proyecto:
#    Procesamos el directorio de nuestra librería de memoria. Esto creará el target 'memory'.
add_subdirectory(lib/memory)

# 3. Entorno de Testing:
#    'enable_testing()' activa el framework de testing de CMake, permitiendo usar CTest.
enable_testing()
#    Procesamos el directorio de tests. Su CMakeLists.txt podrá ahora usar
#    los targets 'unity' y 'memory' que ya hemos definido.
add_subdirectory(test)