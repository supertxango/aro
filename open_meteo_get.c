// --------------------------------------------------------------------------
// Módulo: open_meteo_get
// Descripción: Incluye funciones para la construcción de la URL con las
// variables que se desean obtener de la API de Open-Meteo, solicitan la
// petición y devuelven un JSON
// --------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "open_meteo_get.h"

/* Callback interno de libcurl para ir acumulando los chunks recibidos */
static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    HttpResponseBuffer *mem = (HttpResponseBuffer *)userp;

    char *ptr = realloc(mem->data, mem->size + realsize + 1);
    if (!ptr) {
        // Error de memoria
        return 0; 
    }

    mem->data = ptr;
    memcpy(&(mem->data[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->data[mem->size] = 0; // Terminar en nulo la cadena

    return realsize;
}

/**
 * @brief Realiza una petición GET a la URL indicada y devuelve el cuerpo de la respuesta en una cadena dinámicamente asignada.
 * @param url Cadena con la URL completa.
 * @return Puntero a la cadena de texto con el JSON recibido (debe liberarse con free()), o NULL si ocurre un error.
 */
char* fetch_open_meteo_data(const char *url) {
    if (!url) return NULL;

    CURL *curl_handle = curl_easy_init();
    if (!curl_handle) return NULL;

    HttpResponseBuffer chunk;
    chunk.data = malloc(1); // Se ampliará dinámicamente en write_callback
    chunk.size = 0;

    if (!chunk.data) {
        curl_easy_cleanup(curl_handle);
        return NULL;
    }

    // Configuración de libcurl
    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "Aro-Dagda-Module/1.0");
    curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 10L); // Timeout de 10 segundos

    CURLcode res = curl_easy_perform(curl_handle);

    if (res != CURLE_OK) {
        fprintf(stderr, "[ERROR Dagda API] curl_easy_perform() falló: %s\n", curl_easy_strerror(res));
        free(chunk.data);
        chunk.data = NULL;
    }

    curl_easy_cleanup(curl_handle);
    return chunk.data; // Devuelve la cadena JSON acumulada
}

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
                          char *buffer, size_t buffer_size) {
    if (!buffer || buffer_size == 0) return false;

    int written = snprintf(buffer, buffer_size, 
                           "%s?latitude=%.6f&longitude=%.6f&timezone=auto", 
                           OPEN_METEO_BASE_URL, latitude, longitude);

    if (written < 0 || (size_t)written >= buffer_size) return false;

    if (current_vars && strlen(current_vars) > 0) {
        written += snprintf(buffer + written, buffer_size - written, "&current=%s", current_vars);
        if ((size_t)written >= buffer_size) return false;
    }

    if (hourly_vars && strlen(hourly_vars) > 0) {
        written += snprintf(buffer + written, buffer_size - written, "&hourly=%s", hourly_vars);
        if ((size_t)written >= buffer_size) return false;
    }

    if (daily_vars && strlen(daily_vars) > 0) {
        written += snprintf(buffer + written, buffer_size - written, "&daily=%s", daily_vars);
        if ((size_t)written >= buffer_size) return false;
    }

    if (forecast_days > 0) {
        written += snprintf(buffer + written, buffer_size - written, "&forecast_days=%d", forecast_days);
        if ((size_t)written >= buffer_size) return false;
    }

    if (past_days > 0) {
        written += snprintf(buffer + written, buffer_size - written, "&past_days=%d", past_days);
        if ((size_t)written >= buffer_size) return false;
    }


    return true;
}
