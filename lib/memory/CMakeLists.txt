# CMakeLists.txt para la librería 'memory'

project(memory C)

add_library(memory SHARED
    src/allocator.c
    src/control.c
    src/heap.c
    src/log.c
    src/stats.c
)

# --- AÑADIR ESTA LÍNEA ---
# Enlaza la librería de memoria con 'libdl', que proporciona dlsym.
# También es bueno enlazar con 'pthread' por el uso de __thread.
target_link_libraries(memory PRIVATE dl pthread)

# --- El resto del archivo se mantiene igual ---
target_include_directories(memory
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_include_directories(memory
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_options(memory PRIVATE
    "-Wl,--defsym=malloc=custom_malloc"
    "-Wl,--defsym=free=custom_free"
    "-Wl,--defsym=calloc=custom_calloc"
    "-Wl,--defsym=realloc=custom_realloc"
)