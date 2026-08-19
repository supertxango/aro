PROYECTO ARO (AGRICULTURE & RURAL OPTIMIZATION)

ARO del celta antiguo, significa “Tierra arable” o “Campo de labranza” y es un proyecto para ayudar en la gestión de un entorno 
agrícola en el medio rural, ya sea un terreno cultivable como un vivero de montaña, mediante la automatización de previsiones 
meteorológicas y de cultivo (condiciones para el riego, abonado, estado del suelo.

En principio está pensado con dos módulos principales:

DAGDA
En la mitología celta, el dios del clima, de la agricultura y la abundancia, controlando los elementos y vigilando las tormentas. 
Está pensado como motor de análisis meteorológico y de evaluación de riesgos climáticos en tiempo real diseñado para funcionar en 
entornos de bajo consumo sobre Linux. Se basa en la información ofrecida por Open Meteo a través de su API para recabar y procesar 
datos atmosféricos y emitir alertas tempranas de posibles tormentas severas e incendios forestales, enfocándose en una zona muy 
concreta marcada por unas coordenadas.

Estructura del código fuente:

main.c: Punto de entrada de la aplicación en CLI. Gestiona el procesamiento de argumentos de línea de comandos mediante ‘getopt_long’ 
y despliega el menú interactivo para consultar el tiempo actual o previsiones.

dagda.c: Implementación de las funciones de evaluación meteorológica, alertas de riesgo de incendios (usando la Regla del 30/30/30), 
sistemas de detección de tormentas y seguimiento de DANAs.

dagda.h: Definiciones de estructuras de datos (como métricas de tormenta e índices DANA) y prototipos de funciones para la lógica de 
análisis meteorológico.

open_meteo_parser.c: Funciones para parsear las respuestas JSON devueltas por la API de Open-Meteo mediante la librería cJSON.

open_meteo_parser.h: Prototipos de las funciones del analizador JSON.

open_meteo_get.c: Implementación de las peticiones HTTP GET a la API utilizando libcurl.

open_meteo_get.h: Encabezado con definiciones para la gestión de buffers de memoria dinámicos durante descargas HTTP.

open_meteo_vars.h: Estructuras en C para representar las variables meteorológicas actuales, previsiones diarias y previsiones horarias.

Requisitos: 

Sistema operativo: Linux (Debian, Ubuntu o similar)
Compilador: gcc / make
Librerías: cJSON y libcurl.
En Debian/Ubuntu puedes instalar las dependencias necesarias con: 
$ sudo apt update && sudo apt install build-essential libcurl4-openssl-dev libcjson-dev

Instalación:

Clonar este repositorio
  $ git clone https://github.com/supertxango/aro.git (https://github.com/supertxango/aro.git)
  $ cd aro
Compilar
  $ gcc -Wall -Wextra -std=c99     main.c dagda.c open_meteo_get.c open_meteo_parser.c     -lcurl -lcjson     -o dagda
Ejecución:
  $ ./dagda (para acceder al menú interactivo).
  $ ./dagda [OPCIÓN]

Opciones:

 	-a, --actual   	  Obtiene y muestra el tiempo actual.
 	-p, --prevision 	Obtiene y muestra la previsión a 3 días.
 	-t, --tormenta 	  Evaluación de previsión de tormenta a 2 días.
  -d, --dana	      Evaluación de previsión de entrada de DANA a 14 días.
  -h, --help     	  Muestra este mensaje de ayuda.


VIVERO (En pensamiento…)
Módulo pensado para calcular el riego necesario basándose en la evapotranspiración de referencia y determinar el agua exacta que 
necesita un cultivo, identificar los mejores días para realizar el abonado dependiendo de las condiciones climáticas y conocer el 
estado del suelo de cultivo en general.
