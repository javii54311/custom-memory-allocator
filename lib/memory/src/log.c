/**
 * @file log.c
 * @brief Implementación del sistema de logging a prueba de recursión.
 *
 * Utiliza llamadas al sistema de bajo nivel (open, write, close) para evitar
 * usar la E/S estándar de libc, que podría llamar a malloc y causar un bucle infinito.
 */
#include "log.h"
#include "memory_control.h"
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

// Usamos un file descriptor (int), no un FILE*, para evitar la E/S buferada de libc.
static int log_fd = -1;

/**
 * @brief Inicializa el sistema de registro de eventos usando llamadas de bajo nivel.
 */
void init_memory_log(const char* filename)
{
    close_memory_log(); // Cierra cualquier log anterior para evitar fugas de descriptores.
    if (filename)
    {
        // Abrimos el archivo para escritura, lo creamos si no existe y lo truncamos (borramos contenido).
        // Los permisos 0644 son estándar: el dueño puede leer/escribir, otros solo leer.
        log_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    }
}

/**
 * @brief Cierra el descriptor de archivo del log.
 */
void close_memory_log(void)
{
    if (log_fd != -1)
    {
        close(log_fd);
        log_fd = -1;
    }
}

/**
 * @brief Escribe un evento en el log de forma segura, sin usar malloc.
 */
void log_event(const char* format, ...)
{
    if (log_fd == -1)
    {
        return; // Logging deshabilitado.
    }

    // Creamos un buffer estático en la pila (stack) para no alocar memoria del heap.
    // Esto es CRUCIAL para no caer en la recursión infinita.
    char buffer[256];
    va_list args;

    va_start(args, format);
    // vsnprintf es una versión segura de sprintf que previene desbordamientos de buffer.
    // Escribimos el mensaje formateado en nuestro buffer local.
    int len = vsnprintf(buffer, sizeof(buffer) - 2, format, args);
    va_end(args);

    if (len > 0)
    {
        // Añadimos un salto de línea y el terminador nulo.
        buffer[len] = '\n';
        buffer[len + 1] = '\0';

        // Usamos la llamada al sistema 'write', que es de bajo nivel y no llama a malloc.
        // Esto rompe el ciclo recursivo que causaba el SegFault.
        write(log_fd, buffer, len + 1);
    }
}