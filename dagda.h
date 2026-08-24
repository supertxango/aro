// ---------------------------------------------------------------------------------
// Módulo: dagda
// Descripción: Incluye las funciones para analizar y calcular todas las previsiones
// que pone a nuestra disposición el módulo Dagda del Proyecto ARO. Estas incluyen 
// entre otras el tiempo actual y la previsión a tres días junto con el riesgo de 
// tormenta e incendio, la evaluación en profundidad de posibles tormentas a dos
// días vista y el cálculo de entrada de vaguadas de aire frío (DANAS) a 14 días 
// vista. 
// ---------------------------------------------------------------------------------
#ifndef DAGDA_H
#define DAGDA_H

#include "open_meteo_vars.h"

// Estructura para almacenar los índices convectivos y dinámicos de una hora
/*typedef struct {
    char time[20];
    float cape;
    float lifted_index;
    float wind_speed_10m;
    float wind_speed_500hpa;
    float shear;              // V_500hPa - V_10m
    float z500;               // Geopotential height 500 hPa
    float delta_z500_3h;      // Z500[t] - Z500[t-3]
    int score_tormenta;       // Puntuación total calculada [0 - 10]
} StormMetrics;*/

// Estructura para almacenar los valores de posibilidad de entrada de DANA
typedef struct {
    int score;               // Puntuación acumulada (0 - 12)
    char nivel[16];          // "NULO", "DEBIL", "MODERADO", "ALTO"
    char color[12];          // Código ANSI
    bool embolsamiento_frio; // Indicador de si hay bolsa de aire frío en altura
} ResultadoDANA;

/**
 * @brief Limpia la pantalla de la terminal enviando secuencias de escape ANSI.
 */
void clean_screen(void);

/**
 * @brief Muestra el encabezado visual del módulo Dagda del proyecto ARO en la consola.
 */
void print_banner(void);

/**
 * @brief Evalúa e imprime por consola el nivel de riesgo actual de incendios forestales mediante la regla del 30/30/30.
 * @param datos Puntero constante a la estructura CurrentWeather con las observaciones meteorológicas actuales.
 */
void evaluar_riesgo_incendio_actual(const CurrentWeather *datos);

/**
 * @brief Evalúa e imprime por consola los indicadores barométricos y el riesgo actual de tormentas/chubascos.
 * @param datos Puntero constante a la estructura CurrentWeather con las observaciones meteorológicas actuales.
 */
void evaluar_riesgo_tormentas_actual(const CurrentWeather *datos);

/**
 * @brief Evalúa la regla del 30/30/30 para cada día en la previsión a corto plazo (máximo 3 días).
 * @param daily Puntero constante a la estructura DailyWeather con la previsión diaria.
 * @param riesgo_incendio_out Vector donde se almacenará el número de factores críticos cumplidos (0 a 3) por día.
 */
void evaluar_riesgo_incendio_prevision(const DailyWeather *daily, int *riesgo_incendio_out);

/**
 * @brief Evalúa el índice de inestabilidad y asigna una categoría de riesgo de tormentas para cada día de la previsión.
 * @param daily Puntero constante a la estructura DailyWeather con la previsión diaria.
 * @param puntos_out Vector donde se guardará la puntuación de inestabilidad calculada por día.
 * @param riesgo_tormenta_out Vector donde se guardará el nivel final de riesgo de tormenta (0: Sin riesgo, 1: Moderado, 2: Severo).
 */
void evaluar_riesgo_tormentas_prevision(const DailyWeather *daily, int *puntos_out, int *riesgo_tormenta_out);

/**
 * @brief Ejecuta la acción de consulta y despliegue del tiempo actual e invoca la evaluación de riesgos inmediatos.
 * @param lat Latitud geográfica en grados decimales.
 * @param lon Longitud geográfica en grados decimales.
 */
void action_current_weather(double lat, double lon);

/**
 * @brief Ejecuta la acción de consulta y procesamiento de la previsión meteorológica diaria a 3 días vista.
 * @param lat Latitud geográfica en grados decimales.
 * @param lon Longitud geográfica en grados decimales.
 */
void action_daily_forecast(double lat, double lon);

/**
 * @brief Calcula una puntuación cuantitativa (0 a 12) del riesgo de tormentas organizadas/severas basándose en variables horarias.
 * @param cape Valor de la Energía Potencial Convectiva Disponible (J/kg).
 * @param li Valor del Lifted Index (º C).
 * @param shear Cizalladura vertical del viento entre 500 hPa y 10m (km/h o kt).
 * @param delta_z500 Variación del geopotencial a 500 hPa en un lapso de 3 horas (mgp).
 * @param rh2m Humedad relativa a 2 metros de altura (%).
 * @param rh700 Humedad relativa en el nivel de 700 hPa (%).
 * @return Puntuación acumulada de riesgo de tormenta (entero).
 */
int calcular_riesgo_tormenta(float cape, float li, float shear, float delta_z500, int rh2m, int rh700);

/**
 * @brief Ejecuta la acción de evaluación de riesgo de tormentas horaria a 48 horas e imprime el análisis por pantalla.
 * @param lat Latitud geográfica en grados decimales.
 * @param lon Longitud geográfica en grados decimales.
 */
void action_storm_evaluation(double lat, double lon);

/**
 * @brief Evalúa la probabilidad de desarrollo o presencia de una DANA/Gota Fría analizando la firma en altura y parámetros termodinámicos.
 * @param temperature_500hPa Temperatura del aire en el nivel de 500 hPa (ºC).
 * @param geopotential_height_500hPa Altura geopotencial en 500 hPa (mgp).
 * @param delta_z500_24h Caída o variación de la altura geopotencial a 500 hPa en 24 horas.
 * @param lifted_index Valor del Lifted Index (ºC).
 * @param cape Valor del CAPE (J/kg).
 * @param relative_humidity_700hPa Humedad relativa en 700 hPa (%).
 * @return Estructura ResultadoDANA con la puntuación, nivel de riesgo, flags y color ANSI asignado.
 */
ResultadoDANA calcular_dana_14dias(float temperature_500hPa, float geopotential_height_500hPa, float delta_z500_24h, float lifted_index, float cape, int relative_humidity_700hPa);

/**
 * @brief Consulta la previsión a 14 días en resolución horaria, evalúa la evolución de DANAs y genera un resumen diario de picos de riesgo.
 * @param lat Latitud geográfica en grados decimales.
 * @param lon Longitud geográfica en grados decimales.
 */
void action_dana_evaluation(double lat, double lon);

#endif // DAGDA_H
