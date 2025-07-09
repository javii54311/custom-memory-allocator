/**
 * @file heap.c
 * @brief Implementación de las operaciones de bajo nivel del heap.
 *
 * Este archivo contiene la lógica fundamental para manipular la lista de bloques
 * de memoria: buscar, extender, dividir y fusionar. También define las variables
 * globales que gestionan el estado del heap.
 */
#include "heap.h"
#include "log.h"
#include <sys/mman.h> // Para mmap
#include <unistd.h>   // Para offsetof (a través de stddef.h incluido en unistd.h)

// --- Definición de las Variables Globales ---

/** Puntero al primer bloque de la lista del heap. Inicialmente NULL. */
block_ptr heap_base = NULL;

/** Política de asignación por defecto. */
allocation_policy_t current_policy = FIRST_FIT;

// --- Implementación de Funciones Internas ---

/**
 * @brief Busca un bloque libre en el heap que satisfaga el tamaño requerido.
 * @param last Puntero de salida. Al terminar, apuntará al último bloque visitado.
 * @param size Tamaño de datos (ya alineado) que se necesita.
 * @return Un puntero al bloque encontrado, o NULL si no hay ninguno adecuado.
 */
block_ptr find_free_block(block_ptr* last, size_t size)
{
    block_ptr current = heap_base;
    block_ptr best_fit_block = NULL;
    block_ptr worst_fit_block = NULL;
    size_t min_diff = (size_t)-1; // Inicializado al valor máximo posible
    size_t max_size = 0;

    while (current)
    {
        *last = current; // Mantenemos un registro del último bloque visitado.
        if (current->is_free && current->size >= size)
        {
            switch (current_policy)
            {
            case FIRST_FIT:
                return current; // Primer ajuste: devolver el primero que encontremos.
            case BEST_FIT:
                if (current->size == size)
                    return current; // Ajuste perfecto, no se puede mejorar.
                if (current->size - size < min_diff)
                {
                    min_diff = current->size - size;
                    best_fit_block = current;
                }
                break;
            case WORST_FIT:
                if (current->size > max_size)
                {
                    max_size = current->size;
                    worst_fit_block = current;
                }
                break;
            }
        }
        current = current->next;
    }

    // Al final del bucle, devolvemos el mejor candidato encontrado para Best/Worst fit.
    if (current_policy == BEST_FIT)
        return best_fit_block;
    if (current_policy == WORST_FIT)
        return worst_fit_block;
    return NULL; // Para First Fit, si llegamos aquí, no se encontró nada.
}

/**
 * @brief Pide una nueva región de memoria al sistema operativo usando mmap.
 * @param last El último bloque en la lista, para enlazar el nuevo.
 * @param size El tamaño de datos (ya alineado) para el nuevo bloque.
 * @return Un puntero al nuevo bloque inicializado, o NULL si mmap falla.
 */
block_ptr extend_heap(block_ptr last, size_t size)
{
    size_t total_size = BLOCK_META_SIZE + size;
    block_ptr new_block = mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (new_block == MAP_FAILED)
    {
        log_event("extend_heap: mmap failed");
        return NULL;
    }

    // Inicializamos los metadatos del nuevo bloque.
    new_block->size = size;
    new_block->is_free = false; // Se crea para ser usado inmediatamente.
    new_block->next = NULL;
    new_block->prev = last;
    new_block->data_ptr = new_block->data;

    // Enlazamos el bloque anterior con este nuevo.
    if (last)
    {
        last->next = new_block;
    }

    log_event("extend_heap: Extended heap by %zu bytes at %p", total_size, (void*)new_block);
    return new_block;
}

/**
 * @brief Divide un bloque si es significativamente más grande de lo necesario.
 * @param block El bloque a dividir.
 * @param size El tamaño de datos que se va a ocupar.
 */
void split_block(block_ptr block, size_t size)
{
    // Solo dividimos si el espacio sobrante es suficiente para albergar metadatos y al menos 1 byte de datos.
    if (block->size > size + BLOCK_META_SIZE)
    {
        // Calculamos la dirección del nuevo fragmento libre.
        block_ptr new_fragment = (block_ptr)(block->data + size);
        new_fragment->size = block->size - size - BLOCK_META_SIZE;
        new_fragment->is_free = true;

        // Cosemos el nuevo fragmento en la lista doblemente enlazada.
        new_fragment->next = block->next;
        new_fragment->prev = block;
        new_fragment->data_ptr = new_fragment->data;

        if (block->next)
        {
            block->next->prev = new_fragment;
        }

        // Ajustamos el tamaño del bloque original y lo apuntamos al nuevo fragmento.
        block->size = size;
        block->next = new_fragment;
        log_event("split_block: Split block %p into %zu and %zu bytes", (void*)block, block->size, new_fragment->size);
    }
}

/**
 * @brief Fusiona un bloque con sus vecinos si también están libres.
 * @param block El bloque que se acaba de liberar.
 * @return El puntero al bloque base de la fusión (puede ser el original o su `prev`).
 */
block_ptr coalesce_blocks(block_ptr block)
{
    block_ptr current_block = block;

    // 1. Intenta fusionar con el bloque ANTERIOR.
    if (current_block->prev && current_block->prev->is_free)
    {
        log_event("coalesce: Fusing with prev %p", (void*)current_block->prev);
        block_ptr prev_block = current_block->prev;
        prev_block->size += BLOCK_META_SIZE + current_block->size;
        prev_block->next = current_block->next;
        if (current_block->next)
        {
            current_block->next->prev = prev_block;
        }
        current_block = prev_block; // El nuevo bloque base es el anterior.
    }

    // 2. Intenta fusionar el bloque actual con el SIGUIENTE.
    if (current_block->next && current_block->next->is_free)
    {
        log_event("coalesce: Fusing with next %p", (void*)current_block->next);
        current_block->size += BLOCK_META_SIZE + current_block->next->size;
        current_block->next = current_block->next->next;
        if (current_block->next)
        {
            current_block->next->prev = current_block;
        }
    }

    return current_block;
}

/**
 * @brief Obtiene un puntero a los metadatos de un bloque a partir del puntero de datos del usuario.
 * @param p Puntero devuelto al usuario por malloc.
 * @return Puntero a la estructura s_block correspondiente.
 */
block_ptr get_block_from_ptr(void* p)
{
    // La magia está en restar el offset de los metadatos.
    return (block_ptr)((char*)p - offsetof(struct s_block, data));
}

/**
 * @brief Valida si un puntero pertenece a un bloque de memoria asignado por nuestro gestor.
 * @param p Puntero a validar.
 * @return true si es una dirección válida, false en caso contrario.
 */
bool is_valid_address(void* p)
{
    if (!p || !heap_base)
    {
        return false;
    }
    // La forma segura es recorrer la lista y comprobar si el puntero
    // coincide con el data_ptr de algún bloque ocupado.
    block_ptr current = heap_base;
    while (current)
    {
        if (!current->is_free && current->data_ptr == p)
        {
            return true;
        }
        current = current->next;
    }
    return false;
}

/**
 * @brief Reinicia el heap, abandonando la memoria anterior. ¡SOLO PARA TESTING!
 */
void reset_heap_for_testing(void)
{
    // Esto es un "reseteo duro". Simplemente hacemos que la base del heap apunte a NULL.
    // La memoria anterior asignada con mmap queda "huérfana" (leak), pero para un
    // proceso de test que termina inmediatamente, es una forma aceptable de aislar pruebas.
    heap_base = NULL;
    log_event("====== HEAP RESET FOR NEW TEST ======");
}