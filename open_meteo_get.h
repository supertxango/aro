// --------------------------------------------------------------------------
// Módulo: open_meteo_get
// Descripción: Incluye funciones para la construcción de la URL con las
// variables que se desean obtener de la API de Open-Meteo, solicitan la
// petición y devuelven un JSON
// --------------------------------------------------------------------------

#ifndef OPEN_METEO_GET_H
#define OPEN_METEO_GET_H

#include <stdbool.h>
#include <stddef.h>

#define OPEN_METEO_BASE_URL "https://api.open-meteo.com/v1/forecast"
#define INITIAL_BUFFER_SIZE 4096

// Estructura para almacenar la respuesta HTTP recibida en chunks por libcurl
typedef struct {
    char *data;
    size_t size;
} HttpResponseBuffer;

/**
 * @brief Realiza una petición GET a la URL indicada y devuelve el cuerpo de la respuesta en una cadena dinámicamente asignada.
 * @param url Cadena con la URL completa.
 * @return Puntero a la cadena de texto con el JSON recibido (debe liberarse con free()), o NULL si ocurre un error.
 */
char* fetch_open_meteo_data(const char *url);

/**
 * @brief Construye la URL de consulta para los datos actuales, horarios o diarios.
 * @param latitude Latitud en grados decimales.
 * @param longitude Longitud en grados decimales.
 * @param hourly_vars Cadena con variables horarias separadas por coma (opcional, puede ser NULL).
 * @param daily_vars Cadena con variables diarias separadas por coma (opcional, puede ser NULL).
 * @param current_vars Cadena con variables actuales separadas por coma (opcional, puede ser NULL).
 * @param forecast_days Número de días de predicción (p. ej. 1, 3, 7).
 * @param past_days Número de días pasados a la fecha de petición actual (p. ej. 1, 3, 7).
 * @param buffer Buffer de salida donde se escribirá la URL generada.
 * @param buffer_size Tamaño del buffer asignado.
 * @return true si la URL se construyó correctamente, false si excedió el tamaño del buffer.
 */
bool build_open_meteo_url(double latitude, double longitude, 
                          const char *hourly_vars, 
                          const char *daily_vars, 
                          const char *current_vars, 
                          int forecast_days, 
                          int past_days,
                          char *buffer, size_t buffer_size);

#endif // OPEN_METEO_GET_H
