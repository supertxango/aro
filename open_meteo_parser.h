// ---------------------------------------------------------------------------------
// Módulo: open_meteo_parser
// Descripción: Incluye funciones para procesar el JSON recibido de la petición
// a la API de Open Meteo para añadirlo a las estructuras de datos correspondientes
// que alamcenarán los datos meteorológicos para poder hacer cálculos.
// Adicionalmente incluye las funciones para liberación de memoria una vez acabado
// el proceso de los datos.
// ---------------------------------------------------------------------------------

#ifndef OPEN_METEO_PARSER_H
#define OPEN_METEO_PARSER_H

#include <stdbool.h>
#include "open_meteo_vars.h"

/**
 * @brief Parsea los datos meteorológicos actuales a la estructura CurrentWeather.
 * @param json_string Cadena de texto en formato JSON.
 * @param out_current Puntero a la estructura donde se guardarán los datos.
 * @return true si el parseo fue exitoso, false en caso contrario.
 */
bool parse_current_weather(const char *json_string, CurrentWeather *out_current);

/**
 * @brief Parsea la serie temporal diaria e inicializa memoria dinámica para cada array.
 * @param json_string Cadena de texto en formato JSON.
 * @param out_daily Puntero a la estructura donde se guardarán los datos.
 * @return true si el parseo fue exitoso, false en caso contrario.
 */
bool parse_daily_weather(const char *json_string, DailyWeather *out_daily);

/**
 * @brief Parsea la serie temporal horaria e inicializa memoria dinámica para cada array.
 * @param json_string Cadena de texto en formato JSON.
 * @param out_hourly Puntero a la estructura donde se guardarán los datos.
 * @return true si el parseo fue exitoso, false en caso contrario.
 */
bool parse_hourly_weather(const char *json_string, HourlyWeather *out_hourly);

/**
 * @brief Libera la memoria asignada dinámicamente en DailyWeather.
 */
void free_daily_weather(DailyWeather *d);

/**
 * @brief Libera la memoria asignada dinámicamente en HourlyWeather.
 */
void free_hourly_weather(HourlyWeather *h);

#endif // OPEN_METEO_PARSER_H
