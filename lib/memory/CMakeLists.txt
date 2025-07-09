# CMakeLists.txt para la librería 'memory'
# Este archivo define cómo compilar y empaquetar la librería de gestión de memoria.

# Define un "sub-proyecto" llamado 'memory'. Esto ayuda a organizar la compilación.
# Aunque el proyecto principal se llame 'MemoryManager', este target se llamará 'memory'.
project(memory C)

# --- Creación de la Librería ---
# add_library() es el comando principal para crear una librería.
#
# 'memory': Es el nombre del target. Cuando enlacemos desde otro sitio, usaremos este nombre.
#
# 'SHARED': Especifica que queremos crear una librería compartida (un archivo .so en Linux).
#           Esto es ESENCIAL para poder usar la técnica de LD_PRELOAD, que nos permite
#           interceptar las llamadas a malloc, free, etc. en tiempo de ejecución.
#           Si fuera 'STATIC' (.a), el enlazado sería en tiempo de compilación y la
#           intercepción no funcionaría de la misma manera.
#
# Lista de archivos fuente: Todos los .c que componen la lógica de nuestra librería.
add_library(memory SHARED
    src/allocator.c
    src/control.c
    src/heap.c
    src/log.c
    src/stats.c
)


# --- Gestión de Directorios de Inclusión (Headers) ---

# Hacemos que el directorio 'include' sea PÚBLICO.
# 'PUBLIC' significa que cualquier otro target que se enlace con 'memory' (como nuestros tests)
# automáticamente tendrá acceso a los headers de este directorio.
# Esto permite que los clientes hagan #include <memory_allocator.h> sin problemas.
target_include_directories(memory
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Hacemos que el directorio 'src' sea PRIVADO.
# 'PRIVATE' significa que solo los archivos fuente de la propia librería 'memory'
# pueden ver los headers de este directorio (como 'heap.h' y 'log.h').
# Esto es una buena práctica de encapsulamiento: ocultamos los detalles de
# implementación interna al mundo exterior.
target_include_directories(memory
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)


# --- Magia del Enlazador (Linker): Sobreescritura de Símbolos ---

# target_link_options() permite pasar opciones directamente al enlazador (linker).
# Aquí usamos la opción '-Wl,--defsym=<symbol>=<value>' del enlazador GNU ld.
#
# '-Wl,': Es la forma de pasar una opción al enlazador a través del compilador (gcc).
#
# '--defsym=malloc=custom_malloc': Este comando crea un alias. Le dice al enlazador:
# "Cuando alguien busque el símbolo 'malloc', en su lugar, dale la dirección
#  de mi función 'custom_malloc'".
#
# Hacemos esto para las cuatro funciones estándar. El resultado es que cualquier
# programa que se cargue con nuestra librería .so usando LD_PRELOAD, al intentar
# llamar a 'malloc', será redirigido a nuestra 'custom_malloc' sin necesidad de
# modificar su código fuente. Es una forma de "secuestro de funciones" muy potente.
target_link_options(memory PRIVATE
    "-Wl,--defsym=malloc=custom_malloc"
    "-Wl,--defsym=free=custom_free"
    "-Wl,--defsym=calloc=custom_calloc"
    "-Wl,--defsym=realloc=custom_realloc"
)