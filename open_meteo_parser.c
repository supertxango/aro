#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cjson/cJSON.h>
#include "open_meteo_parser.h"

/* =========================================================================
 * 1. PARSEO DE CURRENT WEATHER
 * ========================================================================= */
bool parse_current_weather(const char *json_string, CurrentWeather *out_current) {
    if (!json_string || !out_current) return false;

    cJSON *root = cJSON_Parse(json_string);
    if (!root) return false;

    cJSON *current = cJSON_GetObjectItemCaseSensitive(root, "current");
    if (!cJSON_IsObject(current)) {
        cJSON_Delete(root);
        return false;
    }

    cJSON *item;
    
    item = cJSON_GetObjectItemCaseSensitive(current, "temperature_2m");
    if (cJSON_IsNumber(item)) out_current->temperature_2m = (float)item->valuedouble;

    item = cJSON_GetObjectItemCaseSensitive(current, "relative_humidity_2m");
    if (cJSON_IsNumber(item)) out_current->relative_humidity_2m = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(current, "apparent_temperature");
    if (cJSON_IsNumber(item)) out_current->apparent_temperature = (float)item->valuedouble;

    //item = cJSON_GetObjectItemCaseSensitive(current, "is_day");
    //if (cJSON_IsNumber(item)) out_current->is_day = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(current, "precipitation");
    if (cJSON_IsNumber(item)) out_current->precipitation = (float)item->valuedouble;

    item = cJSON_GetObjectItemCaseSensitive(current, "rain");
    if (cJSON_IsNumber(item)) out_current->rain = (float)item->valuedouble;

    item = cJSON_GetObjectItemCaseSensitive(current, "showers");
    if (cJSON_IsNumber(item)) out_current->showers = (float)item->valuedouble;

    item = cJSON_GetObjectItemCaseSensitive(current, "weather_code");
    if (cJSON_IsNumber(item)) out_current->weather_code = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(current, "cloud_cover");
    if (cJSON_IsNumber(item)) out_current->cloud_cover = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(current, "pressure_msl");
    if (cJSON_IsNumber(item)) out_current->pressure_msl = (float)item->valuedouble;

    item = cJSON_GetObjectItemCaseSensitive(current, "surface_pressure");
    if (cJSON_IsNumber(item)) out_current->surface_pressure = (float)item->valuedouble;

    item = cJSON_GetObjectItemCaseSensitive(current, "wind_speed_10m");
    if (cJSON_IsNumber(item)) out_current->wind_speed_10m = (float)item->valuedouble;

    item = cJSON_GetObjectItemCaseSensitive(current, "wind_direction_10m");
    if (cJSON_IsNumber(item)) out_current->wind_direction_10m = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(current, "wind_gusts_10m");
    if (cJSON_IsNumber(item)) out_current->wind_gusts_10m = (float)item->valuedouble;

    cJSON_Delete(root);
    return true;
}

/* =========================================================================
 * 2. PARSEO DE DAILY WEATHER
 * ========================================================================= */
bool parse_daily_weather(const char *json_string, DailyWeather *out_daily) {
    if (!json_string || !out_daily) return false;

    cJSON *root = cJSON_Parse(json_string);
    if (!root) return false;

    cJSON *daily = cJSON_GetObjectItemCaseSensitive(root, "daily");
    if (!cJSON_IsObject(daily)) {
        cJSON_Delete(root);
        return false;
    }

	// 1. Obtener el array 'time' del cJSON
    cJSON *time_arr = cJSON_GetObjectItemCaseSensitive(daily, "time");
    if (!cJSON_IsArray(time_arr)) {
        cJSON_Delete(root);
        return false;
    }

    // 2. Declarar 'count' antes de usarlo en el malloc
    int count = cJSON_GetArraySize(time_arr);
    out_daily->count = count;

	// 3. Reservar memoria con 'count' ya declarado
	out_daily->time = malloc(count * sizeof(char *));
	if (!out_daily->time) {
    	cJSON_Delete(root);
    	return false;
	}

	// 4. Bucle para extraer cada fecha usando 'time_arr' y 'count'
	for (int i = 0; i < count; i++) {
    	cJSON *item = cJSON_GetArrayItem(time_arr, i);
   		if (cJSON_IsString(item) && item->valuestring != NULL) {
        	// Reserva exacta + '\0' y copia limpia
        	out_daily->time[i] = malloc(strlen(item->valuestring) + 1);
        	if (out_daily->time[i]) {
            	strcpy(out_daily->time[i], item->valuestring);
        	}
    	} else {
        	out_daily->time[i] = malloc(4);
       		 if (out_daily->time[i]) {
            	strcpy(out_daily->time[i], "N/A");
        	}
    	}
	}

    // Reservas dinámicas
    out_daily->temperature_2m_max = malloc(sizeof(float) * count);
    out_daily->temperature_2m_min = malloc(sizeof(float) * count);
    out_daily->weather_code = malloc(sizeof(int) * count);
    out_daily->rain_sum = malloc(sizeof(float) * count);
    out_daily->showers_sum = malloc(sizeof(float) * count);
    out_daily->precipitation_sum = malloc(sizeof(float) * count);
    out_daily->precipitation_hours = malloc(sizeof(float) * count);
    out_daily->precipitation_probability_max = malloc(sizeof(int) * count);
    out_daily->wind_speed_10m_max = malloc(sizeof(float) * count);
    out_daily->wind_gusts_10m_max = malloc(sizeof(float) * count);
    out_daily->wind_direction_10m_dominant = malloc(sizeof(int) * count);
	out_daily->relative_humidity_2m_min = malloc(sizeof(int) * count);
    out_daily->cape_max = malloc(sizeof(float) * count);
    out_daily->updraft_max = malloc(sizeof(float) * count);


    cJSON *t_max = cJSON_GetObjectItemCaseSensitive(daily, "temperature_2m_max");
    cJSON *t_min = cJSON_GetObjectItemCaseSensitive(daily, "temperature_2m_min");
    cJSON *w_cod = cJSON_GetObjectItemCaseSensitive(daily, "weather_code");
    cJSON *r_sum = cJSON_GetObjectItemCaseSensitive(daily, "rain_sum");
    cJSON *s_sum = cJSON_GetObjectItemCaseSensitive(daily, "showers_sum");
    cJSON *p_sum = cJSON_GetObjectItemCaseSensitive(daily, "precipitation_sum");
    cJSON *p_hrs = cJSON_GetObjectItemCaseSensitive(daily, "precipitation_hours");
    cJSON *p_prb = cJSON_GetObjectItemCaseSensitive(daily, "precipitation_probability_max");
    cJSON *w_max = cJSON_GetObjectItemCaseSensitive(daily, "wind_speed_10m_max");
    cJSON *g_max = cJSON_GetObjectItemCaseSensitive(daily, "wind_gusts_10m_max");
    cJSON *w_dir = cJSON_GetObjectItemCaseSensitive(daily, "wind_direction_10m_dominant");
    cJSON *r_hum = cJSON_GetObjectItemCaseSensitive(daily, "relative_humidity_2m_min");
    cJSON *c_max = cJSON_GetObjectItemCaseSensitive(daily, "cape_max");
    cJSON *u_max = cJSON_GetObjectItemCaseSensitive(daily, "updraft_max");



    for (int i = 0; i < count; i++) {
        if (t_max) out_daily->temperature_2m_max[i] = (float)cJSON_GetArrayItem(t_max, i)->valuedouble;
        if (t_min) out_daily->temperature_2m_min[i] = (float)cJSON_GetArrayItem(t_min, i)->valuedouble;
        if (w_cod) out_daily->weather_code[i] = (float)cJSON_GetArrayItem(w_cod, i)->valueint;
        if (r_sum) out_daily->rain_sum[i] = (float)cJSON_GetArrayItem(r_sum, i)->valuedouble;
        if (s_sum) out_daily->showers_sum[i] = (float)cJSON_GetArrayItem(s_sum, i)->valuedouble;
        if (p_sum) out_daily->precipitation_sum[i] = (float)cJSON_GetArrayItem(p_sum, i)->valuedouble;
        if (p_hrs) out_daily->precipitation_hours[i] = (float)cJSON_GetArrayItem(p_hrs, i)->valuedouble;
        if (p_prb) out_daily->precipitation_probability_max[i] = (float)cJSON_GetArrayItem(p_prb, i)->valueint;
        if (w_max) out_daily->wind_speed_10m_max[i] = (float)cJSON_GetArrayItem(w_max, i)->valuedouble;
        if (g_max) out_daily->wind_gusts_10m_max[i] = (float)cJSON_GetArrayItem(g_max, i)->valuedouble;
        if (w_dir) out_daily->wind_direction_10m_dominant[i] = (float)cJSON_GetArrayItem(w_dir, i)->valueint;
        if (r_hum) out_daily->relative_humidity_2m_min[i] = (int)cJSON_GetArrayItem(r_hum, i)->valueint;
        if (c_max) out_daily->cape_max[i] = (float)cJSON_GetArrayItem(c_max, i)->valuedouble;
        if (u_max) out_daily->updraft_max[i] = (float)cJSON_GetArrayItem(u_max, i)->valuedouble;

    }

    cJSON_Delete(root);
    return true;
}

/* =========================================================================
 * 3. PARSEO DE HOURLY WEATHER
 * ========================================================================= */
bool parse_hourly_weather(const char *json_string, HourlyWeather *out_hourly) {
    if (!json_string || !out_hourly) return false;

    cJSON *root = cJSON_Parse(json_string);
    if (!root) return false;

    cJSON *hourly = cJSON_GetObjectItemCaseSensitive(root, "hourly");
    if (!cJSON_IsObject(hourly)) {
        cJSON_Delete(root);
        return false;
    }

	// 1. Obtener el array 'time' del cJSON
    cJSON *time_arr = cJSON_GetObjectItemCaseSensitive(hourly, "time");
    if (!cJSON_IsArray(time_arr)) {
        cJSON_Delete(root);
        return false;
    }

    // 2. Declarar 'count' antes de usarlo en el malloc
    int count = cJSON_GetArraySize(time_arr);
    out_hourly->count = count;

	// 3. Reservar memoria con 'count' ya declarado
	out_hourly->time = malloc(count * sizeof(char *));
	if (!out_hourly->time) {
    	cJSON_Delete(root);
    	return false;
	}

	// 4. Bucle para extraer cada fecha usando 'time_arr' y 'count'
	for (int i = 0; i < count; i++) {
    	cJSON *item = cJSON_GetArrayItem(time_arr, i);
   		if (cJSON_IsString(item) && item->valuestring != NULL) {
        	// Reserva exacta + '\0' y copia limpia
        	out_hourly->time[i] = malloc(strlen(item->valuestring) + 1);
        	if (out_hourly->time[i]) {
            	strcpy(out_hourly->time[i], item->valuestring);
        	}
    	} else {
        	out_hourly->time[i] = malloc(4);
       		 if (out_hourly->time[i]) {
            	strcpy(out_hourly->time[i], "N/A");
        	}
    	}
	}
 
    out_hourly->temperature_2m = malloc(sizeof(float) * count);
    out_hourly->relative_humidity_2m = malloc(sizeof(int) * count);
    out_hourly->dew_point_2m = malloc(sizeof(float) * count);
    out_hourly->precipitation_probability = malloc(sizeof(int) * count);
    out_hourly->precipitation = malloc(sizeof(float) * count);
    out_hourly->rain = malloc(sizeof(float) * count);
    out_hourly->showers = malloc(sizeof(float) * count);
    out_hourly->weather_code = malloc(sizeof(int) * count);
    out_hourly->pressure_msl = malloc(sizeof(float) * count);
    out_hourly->surface_pressure = malloc(sizeof(float) * count);
    out_hourly->cloud_cover = malloc(sizeof(int) * count);
    out_hourly->cloud_cover_low = malloc(sizeof(int) * count);
    out_hourly->cloud_cover_mid = malloc(sizeof(int) * count);
    out_hourly->cloud_cover_high = malloc(sizeof(int) * count);
    out_hourly->wind_speed_10m = malloc(sizeof(float) * count);
    out_hourly->wind_gusts_10m = malloc(sizeof(float) * count);
    out_hourly->wind_direction_10m = malloc(sizeof(int) * count);
    out_hourly->wet_bulb_temperature_2m = malloc(sizeof(float) * count);
    out_hourly->total_column_integrated_water_vapour = malloc(sizeof(float) * count);
    out_hourly->cape = malloc(sizeof(float) * count);
    out_hourly->lifted_index = malloc(sizeof(float) * count);
    out_hourly->convective_inhibition = malloc(sizeof(float) * count);
    out_hourly->freezing_level_height = malloc(sizeof(float) * count);

    out_hourly->temperature_850hPa = malloc(sizeof(int) * count);
    out_hourly->temperature_700hPa = malloc(sizeof(int) * count);
    out_hourly->temperature_500hPa = malloc(sizeof(int) * count);

    out_hourly->relative_humidity_850hPa = malloc(sizeof(int) * count);
    out_hourly->relative_humidity_700hPa = malloc(sizeof(int) * count);
    out_hourly->relative_humidity_500hPa = malloc(sizeof(int) * count);

    out_hourly->wind_speed_850hPa = malloc(sizeof(float) * count);
    out_hourly->wind_speed_700hPa = malloc(sizeof(float) * count);
    out_hourly->wind_speed_500hPa = malloc(sizeof(float) * count);
    
    out_hourly->wind_direction_850hPa = malloc(sizeof(int) * count);
    out_hourly->wind_direction_700hPa = malloc(sizeof(int) * count);
    out_hourly->wind_direction_500hPa = malloc(sizeof(int) * count);

    out_hourly->geopotential_height_850hPa = malloc(sizeof(float) * count);
    out_hourly->geopotential_height_700hPa = malloc(sizeof(float) * count);
    out_hourly->geopotential_height_500hPa = malloc(sizeof(float) * count);


    cJSON *t2m  = cJSON_GetObjectItemCaseSensitive(hourly, "temperature_2m");
    cJSON *rh2m = cJSON_GetObjectItemCaseSensitive(hourly, "relative_humidity_2m");
    cJSON *dp2m = cJSON_GetObjectItemCaseSensitive(hourly, "dew_point_2m");
    cJSON *pp = cJSON_GetObjectItemCaseSensitive(hourly, "precipitation_probability");
    cJSON *pr = cJSON_GetObjectItemCaseSensitive(hourly, "precipitation");
    cJSON *rn = cJSON_GetObjectItemCaseSensitive(hourly, "rain");
    cJSON *sh = cJSON_GetObjectItemCaseSensitive(hourly, "showers");
    cJSON *wc = cJSON_GetObjectItemCaseSensitive(hourly, "weather_code");
    cJSON *p_msl = cJSON_GetObjectItemCaseSensitive(hourly, "pressure_msl");
    cJSON *p_sur = cJSON_GetObjectItemCaseSensitive(hourly, "surface_pressure");
    cJSON *cc   = cJSON_GetObjectItemCaseSensitive(hourly, "cloud_cover");
    cJSON *cc_l   = cJSON_GetObjectItemCaseSensitive(hourly, "cloud_cover_low");
    cJSON *cc_m   = cJSON_GetObjectItemCaseSensitive(hourly, "cloud_cover_mid");
    cJSON *cc_h   = cJSON_GetObjectItemCaseSensitive(hourly, "cloud_cover_high");
    cJSON *w_spd = cJSON_GetObjectItemCaseSensitive(hourly, "wind_speed_10m");
    cJSON *w_gst = cJSON_GetObjectItemCaseSensitive(hourly, "wind_gusts_10m");
    cJSON *w_dir = cJSON_GetObjectItemCaseSensitive(hourly, "wind_direction_10m");
    cJSON *wbt = cJSON_GetObjectItemCaseSensitive(hourly, "wet_bulb_temperature_2m");        
    cJSON *tciwv = cJSON_GetObjectItemCaseSensitive(hourly, "total_column_integrated_water_vapour");    
    cJSON *cape = cJSON_GetObjectItemCaseSensitive(hourly, "cape");
    cJSON *li   = cJSON_GetObjectItemCaseSensitive(hourly, "lifted_index");
    cJSON *cin  = cJSON_GetObjectItemCaseSensitive(hourly, "convective_inhibition");
    cJSON *fl  = cJSON_GetObjectItemCaseSensitive(hourly, "freezing_level_height");

    //out_pressure->geopotential_height_500hPa = malloc(sizeof(float) * count);

    cJSON *temp850 = cJSON_GetObjectItemCaseSensitive(hourly, "temperature_850hPa");
    cJSON *temp700 = cJSON_GetObjectItemCaseSensitive(hourly, "temperature_700hPa");
    cJSON *temp500 = cJSON_GetObjectItemCaseSensitive(hourly, "temperature_500hPa");

    cJSON *rh850 = cJSON_GetObjectItemCaseSensitive(hourly, "relative_humidity_850hPa");
    cJSON *rh700 = cJSON_GetObjectItemCaseSensitive(hourly, "relative_humidity_700hPa");
    cJSON *rh500 = cJSON_GetObjectItemCaseSensitive(hourly, "relative_humidity_500hPa");

    cJSON *ws850 = cJSON_GetObjectItemCaseSensitive(hourly, "wind_speed_850hPa");       
    cJSON *ws700 = cJSON_GetObjectItemCaseSensitive(hourly, "wind_speed_700hPa");
    cJSON *ws500 = cJSON_GetObjectItemCaseSensitive(hourly, "wind_speed_500hPa");

    cJSON *wd850 = cJSON_GetObjectItemCaseSensitive(hourly, "wind_direction_850hPa");
    cJSON *wd700 = cJSON_GetObjectItemCaseSensitive(hourly, "wind_direction_700hPa");
    cJSON *wd500 = cJSON_GetObjectItemCaseSensitive(hourly, "wind_direction_500hPa");

	cJSON *gh850 = cJSON_GetObjectItemCaseSensitive(hourly, "geopotential_height_850hPa");
    cJSON *gh700 = cJSON_GetObjectItemCaseSensitive(hourly, "geopotential_height_700hPa");
	cJSON *gh500 = cJSON_GetObjectItemCaseSensitive(hourly, "geopotential_height_500hPa");

    for (int i = 0; i < count; i++) {
        if (t2m)   out_hourly->temperature_2m[i] = (float)cJSON_GetArrayItem(t2m, i)->valuedouble;
        if (rh2m)  out_hourly->relative_humidity_2m[i] = cJSON_GetArrayItem(rh2m, i)->valueint;
        if (dp2m)  out_hourly->dew_point_2m[i] = (float)cJSON_GetArrayItem(dp2m, i)->valuedouble;
        if (pp)  out_hourly->precipitation_probability[i] = cJSON_GetArrayItem(pp, i)->valueint;
        if (pr)  out_hourly->precipitation[i] = (float)cJSON_GetArrayItem(pr, i)->valuedouble;
        if (rn)    out_hourly->rain[i] = (float)cJSON_GetArrayItem(rn, i)->valuedouble;
        if (sh)    out_hourly->showers[i] = (float)cJSON_GetArrayItem(sh, i)->valuedouble;
        if (wc)  out_hourly->weather_code[i] = cJSON_GetArrayItem(wc, i)->valueint;
        if (p_msl) out_hourly->pressure_msl[i] = (float)cJSON_GetArrayItem(p_msl, i)->valuedouble;
        if (p_sur) out_hourly->surface_pressure[i] = (float)cJSON_GetArrayItem(p_sur, i)->valuedouble;
        if (cc)    out_hourly->cloud_cover[i] = cJSON_GetArrayItem(cc, i)->valueint;
        if (cc_l)    out_hourly->cloud_cover_low[i] = cJSON_GetArrayItem(cc_l, i)->valueint;
        if (cc_m)    out_hourly->cloud_cover_mid[i] = cJSON_GetArrayItem(cc_m, i)->valueint;
        if (cc_h)    out_hourly->cloud_cover_high[i] = cJSON_GetArrayItem(cc_h, i)->valueint;
        if (w_spd) out_hourly->wind_speed_10m[i] = (float)cJSON_GetArrayItem(w_spd, i)->valuedouble;
        if (w_gst) out_hourly->wind_gusts_10m[i] = (float)cJSON_GetArrayItem(w_gst, i)->valuedouble;
        if (w_dir) out_hourly->wind_direction_10m[i] = cJSON_GetArrayItem(w_dir, i)->valueint;
        if (wbt)  out_hourly->wet_bulb_temperature_2m[i] = (float)cJSON_GetArrayItem(wbt, i)->valuedouble;
        if (tciwv)  out_hourly->total_column_integrated_water_vapour[i] = (float)cJSON_GetArrayItem(tciwv, i)->valuedouble;
        if (cape)  out_hourly->cape[i] = (float)cJSON_GetArrayItem(cape, i)->valuedouble;
        if (li)    out_hourly->lifted_index[i] = (float)cJSON_GetArrayItem(li, i)->valuedouble;
        if (cin)   out_hourly->convective_inhibition[i] = (float)cJSON_GetArrayItem(cin, i)->valuedouble;
        if (fl)   out_hourly->freezing_level_height[i] = (float)cJSON_GetArrayItem(fl, i)->valuedouble;
        if (temp850) out_hourly->temperature_850hPa[i] = (float)cJSON_GetArrayItem(temp850, i)->valuedouble;
        if (temp700) out_hourly->temperature_700hPa[i] = (float)cJSON_GetArrayItem(temp700, i)->valuedouble;
        if (temp500) out_hourly->temperature_500hPa[i] = (float)cJSON_GetArrayItem(temp500, i)->valuedouble;
        if (rh850) out_hourly->relative_humidity_850hPa[i] = cJSON_GetArrayItem(rh850, i)->valueint;
        if (rh700) out_hourly->relative_humidity_700hPa[i] = cJSON_GetArrayItem(rh700, i)->valueint;
        if (rh500) out_hourly->relative_humidity_500hPa[i] = cJSON_GetArrayItem(rh500, i)->valueint;
        if (ws850) out_hourly->wind_speed_850hPa[i] = (float)cJSON_GetArrayItem(ws850, i)->valuedouble;
        if (ws700) out_hourly->wind_speed_700hPa[i] = (float)cJSON_GetArrayItem(ws700, i)->valuedouble;
        if (ws500) out_hourly->wind_speed_500hPa[i] = (float)cJSON_GetArrayItem(ws500, i)->valuedouble;
        if (wd850) out_hourly->wind_direction_850hPa[i] = cJSON_GetArrayItem(wd850, i)->valueint;
        if (wd700) out_hourly->wind_direction_700hPa[i] = cJSON_GetArrayItem(wd700, i)->valueint;
        if (wd500) out_hourly->wind_direction_500hPa[i] = cJSON_GetArrayItem(wd500, i)->valueint;
        if (gh850) out_hourly->geopotential_height_850hPa[i] = (float)cJSON_GetArrayItem(gh850, i)->valuedouble;
        if (gh700) out_hourly->geopotential_height_700hPa[i] = (float)cJSON_GetArrayItem(gh700, i)->valuedouble;
        if (gh500) out_hourly->geopotential_height_500hPa[i] = (float)cJSON_GetArrayItem(gh500, i)->valuedouble;       
    }

    cJSON_Delete(root);
    return true;
}

/* =========================================================================
 * 5. FUNCIONES DE LIBERACIÓN DE MEMORIA
 * ========================================================================= */
void free_daily_weather(DailyWeather *d) {

    if (!d) return;

	if (d->time) {
    	for (int i = 0; i < d->count; i++) {
        	if (d->time[i]) {
            	free(d->time[i]);
        	}
    	}
    	free(d->time);
    	d->time = NULL;
	}
	
    free(d->temperature_2m_max);
    free(d->temperature_2m_min);
	free(d->weather_code);
	free(d->rain_sum);
	free(d->showers_sum);
	free(d->precipitation_sum);
    free(d->precipitation_hours);
	free(d->precipitation_probability_max);
    free(d->wind_speed_10m_max);
    free(d->wind_gusts_10m_max);
    free(d->wind_direction_10m_dominant);
    free(d->relative_humidity_2m_min);
    free(d->cape_max);
    free(d->updraft_max);
    
    d->count = 0;
}

void free_hourly_weather(HourlyWeather *h) {

    if (!h) return;
    
    free(h->temperature_2m);
    free(h->relative_humidity_2m);
    free(h->dew_point_2m);
    free(h->precipitation_probability);
    free(h->precipitation);
    free(h->rain);
	free(h->showers);
    free(h->weather_code);
   	free(h->pressure_msl);
    free(h->surface_pressure);
    free(h->cloud_cover);
    free(h->cloud_cover_low);
    free(h->cloud_cover_mid);
    free(h->cloud_cover_high);
    free(h->wind_speed_10m);
    free(h->wind_gusts_10m);
    free(h->wind_direction_10m);
    free(h->wet_bulb_temperature_2m);
    free(h->total_column_integrated_water_vapour);
    free(h->cape);
    free(h->lifted_index);
    free(h->convective_inhibition);
    free(h->freezing_level_height);

    free(h->temperature_850hPa);
    free(h->temperature_700hPa);
    free(h->temperature_500hPa);
    free(h->relative_humidity_850hPa);
    free(h->relative_humidity_700hPa);
    free(h->relative_humidity_500hPa);
    free(h->wind_speed_850hPa);
    free(h->wind_speed_700hPa);
    free(h->wind_speed_500hPa);
    free(h->wind_direction_850hPa);
    free(h->wind_direction_700hPa);
    free(h->wind_direction_500hPa);
    free(h->geopotential_height_850hPa);
    free(h->geopotential_height_700hPa);
    free(h->geopotential_height_500hPa);
    
    h->count = 0;
}
