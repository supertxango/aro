// ---------------------------------------------------------------------------------
// Módulo: dagda
// Descripción: Incluye las funciones para analizar y calcular todas las previsiones
// que pone a nuestra disposición el módulo Dagda del Proyecto ARO. Estas incluyen 
// entre otras el tiempo actual y la previsión a tres días junto con el riesgo de 
// tormenta e incendio, la evaluación en profundidad de posibles tormentas a dos
// días vista y el cálculo de entrada de vaguadas de aire frío (DANAS) a 14 días 
// vista. 
// ---------------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "open_meteo_get.h"
#include "open_meteo_parser.h"
#include "dagda.h"


// Codigos de color ANSI para consola
#define COLOR_RESET    "\x1b[0m"
#define COLOR_ROJO     "\x1b[31m"
#define COLOR_NARANJA  "\033[38;5;208m"
#define COLOR_AMARILLO "\x1b[33m"
#define COLOR_VERDE    "\x1b[32m"
#define COLOR_AZUL     "\x1b[34m"
#define COLOR_PARPADEO "\x1b[5;31m" // Rojo parpadeante

// Estrcutura para almacenar los extremos y valores máximos/mínimos por día para la evaluación de DANAS
typedef struct {
    char date[11];            // "YYYY-MM-DD"
    float t500_min;
    float z500_min;
    float dz500_max_neg;      // Caída de geopotencial más acusada (mínimo valor)
    float cape_max;
    float li_min;
    int rh700_max;
    int score_max;
    char nivel_max[16];
    char color_max[32];    // Puntero al color ANSI correspondiente al pico de riesgo
} DailySummary;

/**
 * @brief Limpia la pantalla de la terminal enviando secuencias de escape ANSI.
 */
void clean_screen(void) {
	printf("\033[2J\033[H");
}

/**
 * @brief Muestra el encabezado visual del módulo Dagda del proyecto ARO en la consola.
 */
void print_banner(void) {
	clean_screen();
    printf("\n================================================\n");
    printf("   A R O  -  Agriculture & Rural Optimization      \n");
    printf("   Módulo Dagda: Clima y Alertas          \n");
    printf("================================================\n");
}


/**
 * @brief Evalúa e imprime por consola el nivel de riesgo actual de incendios forestales mediante la regla del 30/30/30.
 * @param datos Puntero constante a la estructura CurrentWeather con las observaciones meteorológicas actuales.
 */
void evaluar_riesgo_incendio_actual(const CurrentWeather *datos) {
    printf("================================================\n");
    printf("  1. ANALISIS DE RIESGO DE INCENDIO\n");
    printf("================================================\n");
    int cumplimiento_30 = 0;

    // Evaluamos la regla del 30/30/30
    printf(" Evaluación 'Regla del 30/30/30':\n");
    if (datos->temperature_2m >= 30.0f) {
        printf("  [!] Temp >= 30ºC : " COLOR_ROJO "SI (%.1f ºC)" COLOR_RESET "\n", datos->temperature_2m);
        cumplimiento_30++;
    } else {
        printf("  [OK] Temp < 30ºC : NO (%.1f ºC)\n", datos->temperature_2m);
    }

    if (datos->relative_humidity_2m <= 30) {
        printf("  [!] Humedad <= 30%% : " COLOR_ROJO "SI (%d%%)" COLOR_RESET "\n", datos->relative_humidity_2m);
        cumplimiento_30++;
    } else {
        printf("  [OK] Humedad > 30%% : NO (%d%%)\n", datos->relative_humidity_2m);
    }

    if (datos->wind_speed_10m >= 30.0f) {
        printf("  [!] Viento >= 30Km/h : " COLOR_ROJO "SI (%.1f Km/h)" COLOR_RESET "\n", datos->wind_speed_10m);
        cumplimiento_30++;
    } else {
        printf("  [OK] Viento < 30 Km/h : NO (%.1f Km/h)\n", datos->wind_speed_10m);
    }

    // Clasificación del nivel de riesgo
    if (cumplimiento_30 == 3) {
        printf(COLOR_PARPADEO " ¡ALERTA EXTREMA! Cumplimiento total de la regla del 30.\n"
                              " Riesgo crítico de propagación de incendios. " COLOR_RESET "\n");
    } else if (cumplimiento_30 == 2) {
        printf(COLOR_ROJO " RIESGO MUY ALTO (2/3 factores críticos activados)." COLOR_RESET "\n");
    } else if (cumplimiento_30 == 1) {
        printf(COLOR_AMARILLO " RIESGO MODERADO (1/3 factores críticos activados)." COLOR_RESET "\n");
    } else {
        printf(COLOR_VERDE " RIESGO BAJO (Condiciones estables)." COLOR_RESET "\n");
    }
    printf("================================================\n");
}

/**
 * @brief Evalúa e imprime por consola los indicadores barométricos y el riesgo actual de tormentas/chubascos.
 * @param datos Puntero constante a la estructura CurrentWeather con las observaciones meteorológicas actuales.
 */
void evaluar_riesgo_tormentas_actual(const CurrentWeather *datos) {
    printf("================================================\n");
    printf("  2. SISTEMA DE DETECCIÓN DE TORMENTAS\n");
    printf("================================================\n");
    printf(" Indicadores barométricos y de Precipitación:\n");
    printf(" - Presión Barométrica: %.1f hPa\n", datos->surface_pressure);
    printf(" - Precipitación actual: %.2f mm\n", datos->precipitation);

    if (datos->precipitation > 5.0f || (datos->surface_pressure < 859.0f && datos->relative_humidity_2m > 80)) {
        printf(COLOR_PARPADEO " ¡ALERTA DE TORMENTA SEVERA Y CHUBASCOS!\n"
                              " Caída acusada de presión y precipitación alta activada." COLOR_RESET "\n");
    } else if (datos->precipitation > 0.0f || datos->surface_pressure < 863.0f) {
        printf(COLOR_AZUL " POSIBLES TORMENTAS / CHUBASCOS DÉBILES." COLOR_RESET "\n");
    } else if (datos->surface_pressure >= 868.0f && datos->relative_humidity_2m < 70) {
        printf(COLOR_VERDE " ATMÓSFERA ESTABLE (Sin riesgo de tormentas)." COLOR_RESET "\n");
    } else {
        printf(COLOR_AMARILLO " ATMÓSFERA VARIABLE (Atención a la evolución barométrica)." COLOR_RESET "\n");
    }
    printf("================================================\n");
}

/**
 * @brief Evalúa la regla del 30/30/30 para cada día en la previsión a corto plazo (máximo 3 días).
 * @param daily Puntero constante a la estructura DailyWeather con la previsión diaria.
 * @param riesgo_incendio_out Vector donde se almacenará el número de factores críticos cumplidos (0 a 3) por día.
 */
 void evaluar_riesgo_incendio_prevision(const DailyWeather *daily, int *riesgo_incendio_out) {
    if (!daily || daily->count == 0) return;

    for (int i = 0; i < daily->count && i < 3; i++) {
        int factor_30 = 0;

        if (daily->temperature_2m_max[i] >= 30.0f)     factor_30++;
        if (daily->relative_humidity_2m_min[i] <= 30)  factor_30++;
        if (daily->wind_speed_10m_max[i] >= 30.0f)     factor_30++;

        riesgo_incendio_out[i] = factor_30;
    }
}

/**
 * @brief Evalúa el índice de inestabilidad y asigna una categoría de riesgo de tormentas para cada día de la previsión.
 * @param daily Puntero constante a la estructura DailyWeather con la previsión diaria.
 * @param puntos_out Vector donde se guardará la puntuación de inestabilidad calculada por día.
 * @param riesgo_tormenta_out Vector donde se guardará el nivel final de riesgo de tormenta (0: Sin riesgo, 1: Moderado, 2: Severo).
 */
 void evaluar_riesgo_tormentas_prevision(const DailyWeather *daily, int *puntos_out, int *riesgo_tormenta_out) {
    if (!daily || daily->count == 0) return;

    for (int i = 0; i < daily->count && i < 3; i++) {
        int puntos = 0;

        float lluvia_total = daily->precipitation_sum[i];
        float shower_sum   = daily->showers_sum[i];
        float horas_precip = daily->precipitation_hours[i];
        int prob_precip    = daily->precipitation_probability_max[i];
        float racha_max    = daily->wind_gusts_10m_max[i];

        // --- 1. FACTOR CONVECTIVO (Prevalencia de chubascos) ---
        if (shower_sum > 0.0f) {
            if (lluvia_total > 0.0f && (shower_sum / lluvia_total) >= 0.5f) {
                puntos += 3; // Predominio claro de chubascos convectivos
            } else {
                puntos += 1;
            }
        }

        // --- 2. FACTOR INTENSIDAD HORARIA (mm / hora) ---
        if (horas_precip > 0.0f) {
            float intensidad = lluvia_total / horas_precip;
            if (intensidad >= 4.0f)      puntos += 3; // Tromba de agua / descarga rápida
            else if (intensidad >= 1.5f) puntos += 2; // Lluvia moderada   
            else if (intensidad >= 0.5f) puntos += 1; 
        }       

        // --- 3. FACTOR PROBABILIDAD Y RACHAS DE VIENTO ---
        if (prob_precip >= 75)      puntos += 2; 
        else if (prob_precip >= 40) puntos += 1; 

        if (racha_max >= 50.0f)      puntos += 2; 
        else if (racha_max >= 35.0f) puntos += 1;

        // --- ASIGNACIÓN DE RESULTADOS --- 
        puntos_out[i] = puntos;

        if (puntos >= 6) {
            riesgo_tormenta_out[i] = 2; // ALERTA SEVERA / TORMENTA PROBABLE
        } else if (puntos >= 3) {
            riesgo_tormenta_out[i] = 1; // AVISO DE INESTABILIDAD
        } else {
            riesgo_tormenta_out[i] = 0; // SIN RIESGO SIGNIFICATIVO          
        }
    }       
}

/**
 * @brief Ejecuta la acción de consulta y despliegue del tiempo actual e invoca la evaluación de riesgos inmediatos.
 * @param lat Latitud geográfica en grados decimales.
 * @param lon Longitud geográfica en grados decimales.
 */
 void action_current_weather(double lat, double lon) {
    print_banner();
    printf("\n[+] Consultando datos en tiempo real (Lat: %.4f, Lon: %.4f)...\n", lat, lon);

    // 1. Definición de variables actuales a solicitar a Open-Meteo
    const char *current_vars = "temperature_2m,relative_humidity_2m,precipitation,wind_speed_10m,surface_pressure";

    // 2. Construcción de la URL
    char url[1024];
    if (!build_open_meteo_url(lat, lon, NULL, NULL, current_vars, 1, 0, url, sizeof(url))) {
        fprintf(stderr, "[!] Error: No se pudo construir la URL de Open-Meteo.\n");
        return;
    }

    printf("[+] URL: %s\n", url);

    // 3. Petición HTTP mediante libcurl
    char *json_response = fetch_open_meteo_data(url);
    if (!json_response) {
        fprintf(stderr, "[!] Error: Falló la descarga de datos HTTP.\n");
        return;
    }

    // 4. Parseo del JSON
    CurrentWeather current = {0};
    if (parse_current_weather(json_response, &current)) {
    	printf("\n================================================\n");
        printf(" [OK] CONDICIÓN ACTUAL EXTRAÍDA CON ÉXITO      \n");
    	printf("================================================\n");
        
        // NOTA: Si en tu open_meteo_vars.h renombraste los campos a 'temperature'
        // o 'wind_speed' en lugar de 'temperature_2m', ajústalos aquí.
        printf(" Temperatura:        %.2f ºC\n", current.temperature_2m);
        printf(" Humedad Relativa:   %d %%\n",     current.relative_humidity_2m);
        printf(" Precipitación:      %.2f mm\n",  current.precipitation);
        printf(" Presión:            %.2f hPa\n",  current.surface_pressure);
        printf(" Viento (10m):       %.2f km/h\n", current.wind_speed_10m);
    	printf("================================================\n");

		evaluar_riesgo_incendio_actual(&current);
		evaluar_riesgo_tormentas_actual(&current);

    } else {
        fprintf(stderr, "[!] Error: No se pudo parsear la estructura CurrentWeather.\n");
    }

    // 5. Liberación de la memoria del JSON devuelto por cURL
    free(json_response);
}


/**
 * @brief Ejecuta la acción de consulta y procesamiento de la previsión meteorológica diaria a 3 días vista.
 * @param lat Latitud geográfica en grados decimales.
 * @param lon Longitud geográfica en grados decimales.
 */
 void action_daily_forecast(double lat, double lon) {
    print_banner();
    printf("\n[+] Consultando previsión a 3 días (Lat: %.4f, Lon: %.4f)...\n", lat, lon);

    // 1. Variables a solicitar para la previsión diaria
    const char *daily_vars = "temperature_2m_max,relative_humidity_2m_min,wind_speed_10m_max,"
                            "wind_gusts_10m_max,precipitation_sum,rain_sum,showers_sum,"
                            "precipitation_hours,precipitation_probability_max";

    // Build URL pidiendo 3 días
    char url[1024];
    if (!build_open_meteo_url(lat, lon, NULL, daily_vars, NULL, 3, 0, url, sizeof(url))) {
        fprintf(stderr, "[!] Error al construir la URL de previsión.\n");
        return;
    }

 	printf("[+] URL: %s\n", url);

    // 2. Petición HTTP
    char *json_response = fetch_open_meteo_data(url);
    if (!json_response) {
        fprintf(stderr, "[!] Error en la descarga de datos HTTP.\n");
        return;
    }

    // 3. Parseo de la estructura DailyWeather
    DailyWeather daily = {0};
    if (!parse_daily_weather(json_response, &daily) || daily.count == 0) {
        fprintf(stderr, "[!] Error al parsear la previsión diaria.\n");
        free(json_response);
        return;
    }

    // Arrays para guardar las evaluaciones de riesgo de los 3 días
    int riesgo_incendio[3] = {0};
    int puntos_inestabilidad[3] = {0};
    int riesgo_tormenta[3] = {0};

    // 4. Ejecución de cálculos de riesgo Dagda
    evaluar_riesgo_incendio_prevision(&daily, riesgo_incendio);
    evaluar_riesgo_tormentas_prevision(&daily, puntos_inestabilidad, riesgo_tormenta);

    // 5. Impresión por pantalla
    printf("====================================================================\n");
    printf(" [OK] PREVISIÓN A 3 DÍAS EXTRAÍDA CON ÉXITO\n");
    printf("====================================================================\n");
    
    const char *etiquetas[3] = {"HOY", "MAÑANA", "PASADO MAÑANA"};

    for (int i = 0; i < daily.count && i < 3; i++) {
        printf(" [%s] - Fecha: %s\n", etiquetas[i], daily.time[i]);
        printf(" - Temp. máxima: %.1f ºC	| - Humedad mínima: %d%%\n", 
               daily.temperature_2m_max[i], daily.relative_humidity_2m_min[i]);
        printf(" - Viento máximo: %.1f Km/h	| - Racha máxima: %.1f Km/h\n", 
               daily.wind_speed_10m_max[i], daily.wind_gusts_10m_max[i]);
        printf(" - Precip. Total: %.1f mm	| - Prob. lluvia: %d%%\n", 
               daily.precipitation_sum[i], daily.precipitation_probability_max[i]);
        printf(" - Tipo de lluvia: Escalar: %.1f mm, Convectiva: %.1f mm (%.1f h)\n", 
               daily.rain_sum[i], daily.showers_sum[i], daily.precipitation_hours[i]);
        printf(" - Índice de inestabilidad: %d puntos\n", puntos_inestabilidad[i]);

        // Evaluación IPP
        switch (riesgo_incendio[i]) {
            case 3: 
                printf(COLOR_PARPADEO " [IPP] ¡ALERTA EXTREMA! Riesgo crítico de propagación de incendios. " COLOR_RESET "\n");
                break;
            case 2: 
                printf(COLOR_ROJO " [IPP] RIESGO MUY ALTO (2/3 factores críticos activados)." COLOR_RESET "\n");
                break;
            case 1: 
                printf(COLOR_AMARILLO " [IPP] RIESGO MODERADO (1/3 factores críticos activados)." COLOR_RESET "\n");
                break;
            default: 
                printf(COLOR_VERDE " [IPP] RIESGO BAJO (Condiciones estables)." COLOR_RESET "\n");
                break;
        }

        // Evaluación Tormentas
        switch (riesgo_tormenta[i]) {
            case 2: 
                printf(COLOR_PARPADEO " [TORMENTAS] ¡RIESGO DE TORMENTA SEVERA! " COLOR_RESET "\n");
                break;
            case 1: 
                printf(COLOR_AZUL " [TORMENTAS] RIESGO MODERADO (Inestabilidad / Posibles chubascos)." COLOR_RESET "\n");
                break;
            default: 
                printf(COLOR_VERDE " [TORMENTAS] RIESGO BAJO (Cielos despejados)." COLOR_RESET "\n");
                break;
        }       
        printf("====================================================================\n");      
    }

    // 6. Liberación de memoria
    free_daily_weather(&daily); // Libera los vectores internos dinámicos (malloc/cJSON)
    free(json_response);         // Libera el JSON descargado por cURL
}

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
int calcular_riesgo_tormenta(float cape, float li, float shear, float delta_z500, int rh2m, int rh700) {
    int puntos = 0;

    // 1. Inestabilidad termodinámica (LI)
    if (li < -5.0f)      puntos += 3;
    else if (li < -2.0f) puntos += 2;
    else if (li < 0.0f)  puntos += 1;

    // 2. Energía Convectiva (CAPE)
    if (cape >= 1500.0f)     puntos += 3;
    else if (cape >= 500.0f) puntos += 2;
    else if (cape >= 100.0f) puntos += 1;

    // 3. Forzamiento Dinámico en Altura (Delta Z500 3h)
    if (delta_z500 <= -12.0f)     puntos += 3;
    else if (delta_z500 <= -6.0f) puntos += 2;
    else if (delta_z500 <= -2.0f) puntos += 1;

    // 4. Cizalladura vertical (V_500 - V_10)
    if (shear >= 20.0f)     puntos += 2;
    else if (shear >= 10.0f) puntos += 1;


	// 5. Disponibilidad de Humedad (RH 2m y RH 700hPa) - Máx 1 pt
    // Se requiere humedad adecuada tanto abajo como arriba para sostener la tormenta
    if (rh2m >= 40.0f && rh700 >= 30.0f) {
        puntos += 1;
    }

    return puntos;
}

/**
 * @brief Ejecuta la acción de evaluación de riesgo de tormentas horaria a 48 horas e imprime el análisis por pantalla.
 * @param lat Latitud geográfica en grados decimales.
 * @param lon Longitud geográfica en grados decimales.
 */
 void action_storm_evaluation(double lat, double lon) {
    print_banner();
    printf("\n[+] Consultando previsión a 48 horas (Lat: %.4f, Lon: %.4f)...\n", lat, lon);

    // 1. Variables a solicitar para la previsión horaria y de valores de  presion
    const char *hourly_vars = "cape,lifted_index,wind_speed_10m,wind_speed_500hPa,geopotential_height_500hPa,relative_humidity_2m,relative_humidity_700hPa";
    //const char *pressure_vars = "wind_speed_500hPa,geopotential_height_500hPa"
 
    // Construir la URL pidiendo 48 horas
    char url[1024];
    if (!build_open_meteo_url(lat, lon, hourly_vars, NULL, NULL, 2, 0, url, sizeof(url))) {
        fprintf(stderr, "[!] Error al construir la URL de previsión a 48 horas.\n");
        return;
    }

 	printf("[+] URL: %s\n", url);

    // 2. Petición HTTP
    char *json_response = fetch_open_meteo_data(url);
    if (!json_response) {
        fprintf(stderr, "[!] Error en la descarga de datos HTTP.\n");
        return;
    }

    // 3. Parseo de la estructura HourlyWeather
    HourlyWeather hourly = {0};
    if (!parse_hourly_weather(json_response, &hourly) || hourly.count == 0) {
        fprintf(stderr, "[!] Error al parsear la previsión a 48 horas.\n");
        free(json_response);
        return;
    }

    printf("==============================================================================================\n");
    printf(" [OK] PREVISIÓN A 2 DÍAS EXTRAÍDA CON ÉXITO\n");
    printf("==============================================================================================\n");
	// Ejemplo de salida procesando los datos extraídos:
	printf("%-16s | %-6s | %-5s | %-4s | %-11s | %-6s | %-8s | %-28s\n", 
       "Fecha / Hora", "CAPE", "LI", "HR", "HR(700hPa)", "Shear", "ΔZ500", "Riesgo Tormenta");
	printf("-----------------+--------+-------+------+-------------+--------+---------+-------------------\n");
	
    for (int i = 0; i < hourly.count && i < 48; i++) {

		float shear = hourly.wind_speed_500hPa[i] - hourly.wind_speed_10m[i];
		
       	float delta_z3h = (i >= 3) ? (hourly.wind_speed_500hPa[i] - hourly.wind_speed_500hPa[i-3]) : 0.0f;

       	int score = calcular_riesgo_tormenta(hourly.cape[i], hourly.lifted_index[i], shear, delta_z3h,hourly.relative_humidity_2m[i], hourly.relative_humidity_700hPa[i]);

       	char *nivel = "ESTABLE";
       	if (score >= 8)      nivel = "SEVERA";
       	else if (score >= 6) nivel = "ORGANIZADA";
       	else if (score >= 4) nivel = "DÉBIL";

		// 1. Determinar el color según la subcadena o el score directamente
		const char *color = COLOR_RESET; // O "" si no tienes reset por defecto

		if (strstr(nivel, "SEVERA")) {
   			color = COLOR_PARPADEO;
		} else if (strstr(nivel, "ORGANIZADA")) {
    		color = COLOR_ROJO;
		} else if (strstr(nivel, "DÉBIL")) {
    		color = COLOR_AMARILLO;
		}

		// Cadena con el nivel de riesgo y la puntuación para imprimir.
		char texto_riesgo[32];
		snprintf(texto_riesgo, sizeof(texto_riesgo), "%s (%d/12)", nivel, score);

		// 2. Un único printf usando la variable de color
		printf("%s%-16s | %6.0f | %5.1f | %4d | %11d | %6.1f | %7.1f | %-28s" COLOR_RESET "\n",
       		color, 
       		hourly.time[i], 
       		hourly.cape[i], 
       		hourly.lifted_index[i], 
       		hourly.relative_humidity_2m[i], 
       		hourly.relative_humidity_700hPa[i], 
       		shear, 
       		delta_z3h, 
       		texto_riesgo);

	}
    // 6. Liberación de memoria
    free_hourly_weather(&hourly); // Libera los vectores internos dinámicos (malloc/cJSON)
    free(json_response);         // Libera el JSON descargado por cURL

}

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
ResultadoDANA evaluar_riesgo_dana(float temperature_500hPa, float geopotential_height_500hPa, float delta_z500_24h, float lifted_index, float cape, int relative_humidity_700hPa) {
    ResultadoDANA res = {0};
    
    // -------------------------------------------------------------
    // 1. Detección de la firma de DANA en Altura (500 hPa)
    // -------------------------------------------------------------
    // Aire muy frío en altura (embolsamiento)
    if (temperature_500hPa <= -22.0f) {
        res.score += 3;
        res.embolsamiento_frio = true;
    } else if (temperature_500hPa <= -16.0f) {
        res.score += 2;
        res.embolsamiento_frio = true;
    } else if (temperature_500hPa <= -12.0f) {
        res.score += 1;
    }

	// Caída o depresión severa de Geopotencial (Vaguada / Aislamiento)
    if (geopotential_height_500hPa < 5400.0f || delta_z500_24h <= -100.0f) {
        res.score += 3;
    } else if (geopotential_height_500hPa < 5520.0f || delta_z500_24h <= -60.0f) {
        res.score += 2;
    } else if (delta_z500_24h <= -30.0f) {
        res.score += 1;
    }

	// -------------------------------------------------------------
    // 2. Inestabilidad Termodinámica (Soporte en Capas Bajas/Medias)
    // -------------------------------------------------------------
    // Lifted Index (Gradiente térmico)
    if (lifted_index <= -5.0f) {
        res.score += 3;
    } else if (lifted_index <= -2.0f) {
        res.score += 2;
    } else if (lifted_index < 0.0f) {
        res.score += 1;
    }

    // CAPE (Energía Disponible)
    if (cape >= 1000.0f) {
        res.score += 2;
    } else if (cape >= 300.0f) {
        res.score += 1;
    }

	// -------------------------------------------------------------
    // 3. Humedad y Aporte de Agua Preciptable (700 hPa)
    // -------------------------------------------------------------
    if (relative_humidity_700hPa >= 80) {
        res.score += 1;
    }

	// -------------------------------------------------------------
    // 4. Clasificación del Riesgo y Colores
    // -------------------------------------------------------------
    if (res.score >= 8 && res.embolsamiento_frio) {
        snprintf(res.nivel, sizeof(res.nivel), "MUY ALTO");
        snprintf(res.color, sizeof(res.color), "%s", COLOR_ROJO);
    } else if (res.score >= 6) {
        snprintf(res.nivel, sizeof(res.nivel), "ALTO");
        snprintf(res.color, sizeof(res.color), "%s", COLOR_NARANJA);
    } else if (res.score >= 4) {
        snprintf(res.nivel, sizeof(res.nivel), "MODERADO");
        snprintf(res.color, sizeof(res.color), "%s", COLOR_AMARILLO);
    } else if (res.score >= 2) {
        snprintf(res.nivel, sizeof(res.nivel), "DEBIL");
        snprintf(res.color, sizeof(res.color), "%s", COLOR_VERDE);
    } else {
        snprintf(res.nivel, sizeof(res.nivel), "NULO");
        snprintf(res.color, sizeof(res.color), "%s", COLOR_RESET);
    }

    return res;
}

/**
 * @brief Consulta la previsión a 14 días en resolución horaria, evalúa la evolución de DANAs y genera un resumen diario de picos de riesgo.
 * @param lat Latitud geográfica en grados decimales.
 * @param lon Longitud geográfica en grados decimales.
 */
void action_dana_evaluation(double lat, double lon) {
    print_banner();
    printf("\n[+] Consultando previsión a 14 días (Lat: %.4f, Lon: %.4f)...\n", lat, lon);

    // 1. Variables a solicitar para la previsión horaria y de valores de  presion
    const char *hourly_vars = "cape,lifted_index,temperature_500hPa,geopotential_height_500hPa,relative_humidity_700hPa";
    //const char *pressure_vars = "wind_speed_500hPa,geopotential_height_500hPa"
 
    // Construir la URL pidiendo 48 horas
    char url[1024];
    if (!build_open_meteo_url(lat, lon, hourly_vars, NULL, NULL, 14, 1, url, sizeof(url))) {
        fprintf(stderr, "[!] Error al construir la URL de previsión a 14 días.\n");
        return;
    }

 	printf("[+] URL: %s\n", url);

    // 2. Petición HTTP
    char *json_response = fetch_open_meteo_data(url);
    if (!json_response) {
        fprintf(stderr, "[!] Error en la descarga de datos HTTP.\n");
        return;
    }

    // 3. Parseo de la estructura HourlyWeather
    HourlyWeather hourly = {0};
    if (!parse_hourly_weather(json_response, &hourly) || hourly.count == 0) {
        fprintf(stderr, "[!] Error al parsear la previsión a 48 horas.\n");
        free(json_response);
        return;
    }

    printf("============================================================================================\n");
    printf(" [OK] PREVISIÓN A 14 DÍAS EXTRAÍDA CON ÉXITO\n");
    printf("============================================================================================\n");

	printf("============================================================================================\n");
	printf("%-17s | %-6s | %-5s | %-7s | %-6s | %-6s | %-3s | %-18s\n","Fecha/Hora","T500","Z500","ΔZ500", "CAPE","LI","RH700","RIESGO DANA (SCORE)"); 
 	printf("------------------+--------+-------+--------+--------+--------+-------+---------------------\n");

	// Estructuras para acumulación diaria (máximo 14 días)
    DailySummary daily[14];
    int day_count = 0;
	
    for (int i = 0; i < hourly.count && i < 336; i++) {

		float delta_z500_24h = (i >= 24) ? (hourly.geopotential_height_500hPa[i] - hourly.geopotential_height_500hPa[i-24]) : 0.0f;

       	ResultadoDANA dana = evaluar_riesgo_dana(hourly.temperature_500hPa[i], hourly.geopotential_height_500hPa[i], delta_z500_24h, hourly.lifted_index[i], hourly.cape[i],hourly.relative_humidity_700hPa[i]);

		// Cadena con el nivel y puntuación de la DANA para imprimir.
		char texto_dana[32];
		snprintf(texto_dana, sizeof(texto_dana), "%s (%2d/12)", dana.nivel, dana.score);

		//float shear = hourly.wind_speed_500hPa[i] - hourly.wind_speed_10m[i];


		printf("%-17s | %6.1f | %5.0f | %6.0f | %6.0f | %6.1f |  %3d%% | %s%-18s%s\n",
           hourly.time[i],
           hourly.temperature_500hPa[i],
           hourly.geopotential_height_500hPa[i],
           delta_z500_24h,
           hourly.cape[i],
           hourly.lifted_index[i],
           hourly.relative_humidity_700hPa[i],
           dana.color,
           texto_dana,
           COLOR_RESET);		

		// --- ACUMULACIÓN DE DATO DIARIO ---
        // Extraer la fecha "YYYY-MM-DD" de "YYYY-MM-DDTHH:MM"
        char current_date[11];
        strncpy(current_date, hourly.time[i], 10);
        current_date[10] = '\0';

		// Detectar si es el inicio de un nuevo día
        if (i % 24 == 0) {
            if (day_count < 14) {
                strncpy(daily[day_count].date, current_date, 11);
                daily[day_count].t500_min = hourly.temperature_500hPa[i];
                daily[day_count].z500_min = hourly.geopotential_height_500hPa[i];
                daily[day_count].dz500_max_neg = delta_z500_24h;
                daily[day_count].cape_max = hourly.cape[i];
                daily[day_count].li_min = hourly.lifted_index[i];
                daily[day_count].rh700_max = hourly.relative_humidity_700hPa[i];
                daily[day_count].score_max = dana.score;
                strncpy(daily[day_count].nivel_max, dana.nivel, sizeof(daily[day_count].nivel_max));
 				strncpy(daily[day_count].color_max, dana.color, sizeof(daily[day_count].color_max));               
 				day_count++;
            }        
		} else {
            // Actualizar extremos del día en curso
			int idx = day_count - 1;
            if (hourly.temperature_500hPa[i] < daily[idx].t500_min) daily[idx].t500_min = hourly.temperature_500hPa[i];
            if (hourly.geopotential_height_500hPa[i] < daily[idx].z500_min) daily[idx].z500_min = hourly.geopotential_height_500hPa[i];
            if (delta_z500_24h < daily[idx].dz500_max_neg) daily[idx].dz500_max_neg = delta_z500_24h;
            if (hourly.cape[i] > daily[idx].cape_max) daily[idx].cape_max = hourly.cape[i];
           	if (hourly.lifted_index[i] < daily[idx].li_min) daily[idx].li_min = hourly.lifted_index[i];
   			if (hourly.relative_humidity_700hPa[i] > daily[idx].rh700_max) daily[idx].rh700_max = hourly.relative_humidity_700hPa[i];

			// Actualizar el pico máximo de riesgo del día
            if (dana.score > daily[idx].score_max) {
            	daily[idx].score_max = dana.score;
             	strncpy(daily[idx].nivel_max, dana.nivel, sizeof(daily[idx].nivel_max));
              	strncpy(daily[idx].color_max, dana.color, sizeof(daily[idx].color_max));
           	}
		}
	}

	// ============================================================================================
    // IMPRESIÓN DEL RESUMEN DIARIO AL FINAL
    // ============================================================================================
    printf("\n\n");
    printf("====================================================================================================\n");
    printf(" [RESUMEN DIARIO] VALORES EXTREMOS Y PICO DE RIESGO A 14 DÍAS\n");
    printf("====================================================================================================\n");
    printf("%-10s | %-8s | %-7s | %-7s | %-8s | %-6s | %-10s | %-18s\n",
           "Fecha", "T500 Min", "Z500 Min", "ΔZ500 Min", "CAPE Max", "LI Min", "RH700 Max", "PICO RIESGO (SCORE)");
    printf("-----------+----------+----------+-----------+----------+--------+------------+---------------------\n");

	for (int d = 0; d < day_count; d++) {
        char texto_dana_daily[32];
        snprintf(texto_dana_daily, sizeof(texto_dana_daily), "%s (%2d/12)", daily[d].nivel_max, daily[d].score_max);

        printf("%-10s | %8.1f | %8.0f | %9.0f | %8.0f | %6.1f |       %3d%% | %s%-18s%s\n",
               daily[d].date,
               daily[d].t500_min,
               daily[d].z500_min,
               daily[d].dz500_max_neg,
               daily[d].cape_max,
               daily[d].li_min,
               daily[d].rh700_max,
               daily[d].color_max,
               texto_dana_daily,
               COLOR_RESET);
    }
    printf("====================================================================================================\n");

	// 6. Liberación de memoria
    free_hourly_weather(&hourly); // Libera los vectores internos dinámicos (malloc/cJSON)
    free(json_response);         // Libera el JSON descargado por cURL

}

















