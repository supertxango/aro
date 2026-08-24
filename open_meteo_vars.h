#ifndef OPEN_METEO_VARS_H
#define OPEN_METEO_VARS_H

#include <stdbool.h>
#include <time.h>

// --------------------------------------------------------------------------
// 1. Variables para almacenar datos meteorológicos actuales (Current Weather)
// ---------------------------------------------------------------------------
typedef struct {
    float temperature_2m;              	// Temperatura a 2m (°C)
    int relative_humidity_2m;          	// Humedad relativa a 2m (%)
    float apparent_temperature;         // Temperatura a 2m (°C)
    float precipitation;               	// Precipitación actual (mm)
    float rain;                        	// Lluvia (mm)
    float showers;                     	// Chubascos / precipitación convectiva (mm)
    int weather_code;                  	// Código de tiempo WMO
    int cloud_cover;                   	// Cobertura nubosa total (%)
    float pressure_msl;                	// Presión a nivel del mar (hPa)
    float surface_pressure;            	// Presión en superficie (hPa)
    float wind_speed_10m;              	// Velocidad del viento a 10m (km/h)
    int wind_direction_10m;            	// Dirección del viento a 10m (grados)
    float wind_gusts_10m;              	// Rachas de viento a 10m (km/h)
    time_t timestamp;                  	// Hora del dato
} CurrentWeather;

// ------------------------------------------------------------------------
// 2. Variables para almacenar datos de predicción horaria (Hourly Weather)
// ------------------------------------------------------------------------
typedef struct {
	int count;						   	// Número total de horas incluidos en la serie
    char **time;                  		// Fecha y hora del intervalo
    float *temperature_2m;
    int *relative_humidity_2m;
    float *dew_point_2m;
    int *precipitation_probability;
    float *precipitation;
    float *rain;
    float *showers;
    int *weather_code;
    float *pressure_msl;
    float *surface_pressure;
    int *cloud_cover;
    int *cloud_cover_low;
    int *cloud_cover_mid;
    int *cloud_cover_high;
    float *wind_speed_10m;
    float *wind_gusts_10m;
    int *wind_direction_10m;

    // Nivel 850 hPa (~1500m)
    float *temperature_850hPa;
    int *relative_humidity_850hPa;
    float *wind_speed_850hPa;
    int *wind_direction_850hPa;
    float *geopotential_height_850hPa;

    // Nivel 700 hPa (~3000m)
    float *temperature_700hPa;
    int *relative_humidity_700hPa;
    float *wind_speed_700hPa;
    int *wind_direction_700hPa;
    float *geopotential_height_700hPa;

    // Nivel 500 hPa (~5600m)
    float *temperature_500hPa;
    int *relative_humidity_500hPa;
    float *wind_speed_500hPa;
    int *wind_direction_500hPa;
    float *geopotential_height_500hPa;    

    // Variables adicionales para tormentas / convección
    float *wet_bulb_temperature_2m;
    float *total_column_integrated_water_vapour;
    float *cape;                         // CAPE (J/kg)
    float *lifted_index;                 // Lifted Index
    float *convective_inhibition;        // CIN (J/kg)
    float *freezing_level_height;        // Altura nivel de congelación (m)
} HourlyWeather;

// -------------------------------------------------------------------
// 3. Variables para almacenar datos de resumen diario (Daily Weather)
// -------------------------------------------------------------------
typedef struct {
	int count;						   	// Número total de días incluidos en la serie
    char **time;                        // Cadena fecha "YYYY-MM-DD"
	float *temperature_2m_max;			// Temperatura máxima del día en ºC
	float *temperature_2m_min;			// Temperatura mínima del dia en ºC
    int *weather_code;
    float *rain_sum;                     // Suma de lluvia en mm
    float *showers_sum;                  // Suma de chubascos en mm
    float *precipitation_sum;            // Suma total de precipitación (mm)
    float *precipitation_hours;          // Horas de precipitación
    int *precipitation_probability_max;  // Probabilidad máx (%)
    float *wind_speed_10m_max;           // Viento máximo (km/h)
    float *wind_gusts_10m_max;           // Racha máxima (km/h)
    int *wind_direction_10m_dominant;    // Dirección dominante
    int *relative_humidity_2m_min;	 // Humedad relativa mínima del día
    
    // Adicionales convección
    float *cape_max;                     // CAPE máximo diario
    float *updraft_max;                  // Corriente ascendente máxima
} DailyWeather;

// --------------------------------------------------------------------------------
// 4. Variables para almacenar datos de alta resolución de 15 minutos (Minutely 15)
// --------------------------------------------------------------------------------
typedef struct {
	int count;						   	// Número total de periodos de 15 minutos incluidos en la serie
    time_t timestamp;                   // Fecha y hora exacta del intervalo
    float *lightning_potential_index;    // Potencial de rayos LPI (J/kg)
} Minutely15Weather;

#endif // OPEN_METEO_VARS_H
