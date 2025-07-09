/**
 * @file log.h
 * @brief Declaración interna para la función de logging.
 *
 * Este es un header PRIVADO, diseñado para ser incluido únicamente por
 * otros archivos fuente (.c) dentro de la librería de memoria.
 * Define el prototipo de la función de registro de eventos.
 */
#ifndef LOG_H
#define LOG_H

#include <stdarg.h> // Necesario para el uso de '...' (argumentos variables)

/**
 * @brief Escribe un mensaje formateado en el archivo de log si está habilitado.
 *
 * Esta función es segura para ser llamada desde dentro del asignador de memoria,
 * ya que utiliza llamadas al sistema de bajo nivel y no provoca nuevas
 * asignaciones de memoria que puedan causar recursión infinita.
 *
 * @param format Una cadena de formato estilo printf.
 * @param ... Argumentos variables correspondientes a la cadena de formato.
 */
void log_event(const char* format, ...);

#endif // LOG_H