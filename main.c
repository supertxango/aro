#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <curl/curl.h>
#include "dagda.h"

// Coordenadas de prueba (Madrid)
#define DEFAULT_LAT  41.119999
#define DEFAULT_LON -3.390000


void print_help(const char *prog_name);
void interactive_menu(double lat, double lon);

int main(int argc, char *argv[]) {
    // Inicialización global de libcurl al arrancar la aplicación
    curl_global_init(CURL_GLOBAL_DEFAULT);

    double lat = DEFAULT_LAT;
    double lon = DEFAULT_LON;

    static struct option long_options[] = {
        {"actual", no_argument, 0, 'a'},
        {"prevision", no_argument, 0, 'p'},
        {"tormenta", no_argument, 0, 't'},
		{"dana", no_argument, 0, 'd'},
        {"help",   no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    // Si se ejecuta sin argumentos, abre el menú interactivo
    if (argc == 1) {
        interactive_menu(lat, lon);
        curl_global_cleanup();
        return EXIT_SUCCESS;
    }

    int opt;
    int option_index = 0;

    while ((opt = getopt_long(argc, argv, "aptdh", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'a':
                action_current_weather(lat, lon);
                break;
            case 'p':
            	action_daily_forecast(lat, lon);
            	break;
            case 't':
            	action_storm_evaluation(lat, lon);
            	break;
            case 'd':
            	action_dana_evaluation(lat, lon);
            	break;
            case 'h':
            default:
                print_help(argv[0]);
                break;
        }
    }

    // Limpieza global de libcurl al salir
    curl_global_cleanup();
    return EXIT_SUCCESS;
}


void print_help(const char *prog_name) {
    printf("Uso: %s [OPCIÓN]\n\n", prog_name);
    printf("Opciones:\n");
    printf("  -a, --actual   	Obtiene y muestra el tiempo actual.\n");
    printf("  -p, --prevision 	Obtiene y muestra la previsión a 3 días.\n");
    printf("  -t, --tormenta 	Evaluación de previsión de tormenta a 2 días.\n");
    printf("  -d, --dana	 	Evaluación de previsión de entrada de DANA a 14 días.\n");
    printf("  -h, --help     	Muestra este mensaje de ayuda.\n\n");
}

void interactive_menu(double lat, double lon) {
    int option = 0;

    do {
        print_banner();
        printf(" 1. Ver condición meteorológica actual.\n");
		printf(" 2. Ver previsión a 3 días y análisis de riesgo.\n");
		printf(" 3. Evaluación de previsión de tormanta a 2 días.\n");
		printf(" 4. Evaluación de previsión de entrada de DANA a 14 días.\n");
        printf(" 5. Salir\n");
        printf("---------------------------------------------------\n");
        printf(" Seleccione una opción [1-5]: ");

        if (scanf("%d", &option) != 1) {
            while (getchar() != '\n'); // Limpiar buffer en entrada no numérica
            option = 0;
            continue;
        }

        switch (option) {
            case 1:
                action_current_weather(lat, lon);
                break;
            case 2:
            	action_daily_forecast(lat, lon);
            	break;
            case 3:
            	action_storm_evaluation(lat, lon);
            	break;
           case 4:
            	action_dana_evaluation(lat, lon);
            	break;
            case 5:
                printf("\nSaliendo de ARO. ¡Hasta pronto!\n\n");
                break;
            default:
                printf("\n[!] Opción no válida.\n");
                break;
        }

        if (option != 5) {
            printf("\nPresione Enter para continuar...");
            while (getchar() != '\n');
            getchar();
        }

    } while (option != 5);
}
