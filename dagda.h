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

void clean_screen(void);
void print_banner(void);
void evaluar_riesgo_incendio_actual(const CurrentWeather *datos);
void evaluar_riesgo_tormentas_actual(const CurrentWeather *datos);
void evaluar_riesgo_incendio_prevision(const DailyWeather *daily, int *riesgo_incendio_out);
void evaluar_riesgo_tormentas_prevision(const DailyWeather *daily, int *puntos_out, int *riesgo_tormenta_out);
void action_current_weather(double lat, double lon);
void action_daily_forecast(double lat, double lon);
// Lógica de cálculo de riesgo de tormenta/granizo
int calcular_riesgo_tormenta(float cape, float li, float shear, float delta_z500, int rh2m, int rh700);
// Nueva función de evaluación severa de tormentas
void action_storm_evaluation(double lat, double lon);
// Lógica de cálculo de riesgo de tormenta/granizo
ResultadoDANA calcular_dana_14dias(float temperature_500hPa, float geopotential_height_500hPa, float delta_z500_24h, float lifted_index, float cape, int relative_humidity_700hPa);
// Nueva función de evaluación severa de tormentas
void action_dana_evaluation(double lat, double lon);

#endif // DAGDA_H
