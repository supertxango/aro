ARO del celta antiguo, significa “Tierra arable” o “Campo de labranza” y es un proyecto para ayudar en la gestión de un entorno 
agrícola en el medio rural, ya sea un terreno cultivable como un vivero de montaña, mediante la automatización de previsiones 
meteorológicas y de cultivo (condiciones para el riego, abonado, estado del suelo.

En principio está pensado con dos módulos principales:

1. DAGDA
En la mitología celta, el dios del clima, de la agricultura y la abundancia, controlando los elementos y vigilando las tormentas. 
Está pensado como motor de análisis meteorológico y de evaluación de riesgos climáticos en tiempo real diseñado para funcionar en 
entornos de bajo consumo sobre Linux. Se basa en la información ofrecida por Open Meteo a través de su API para recabar y procesar 
datos atmosféricos y emitir alertas tempranas de posibles tormentas severas e incendios forestales, enfocándose en una zona muy 
concreta marcada por unas coordenadas.

1.1 Dagda Analysis

Este módulo accede la API de Open Meteo para obtener y parsear en tiempo real los valores necesarios para calcular:
	El tiempo actual con el cáculo de riesgos de incendio y de tormentas.
	Previsión del tiempo a tres días con el cáculo de riesgos de incendio y de tormentas.
	Evaluación de tormentas en las siguientes 48 horas.
	Evaluación de posibles DANAs en los siguientes 14 días.

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


1.2 Dagda Agent

Este módulo genera un boletín de las alertas relacionadas con riesgo de tormentas y de incendios, así como recomendaciones de trabajos en el vivero, para los siguientes 14 días.
Para ello utiliza los análisis realizados por Dagda Analysis y los comparte mediante un prompt de Gemini que actua como Agente Meteorológico y que es quien realiza
el boletín meteorológico, que queda almacenado en un fichero y lo envía por correo.

Estructura del código fuente:

run_dagda.sh: Genera un fichero de log con la salida del análisis de Dagda (tiempo actual, previsión a tres días, evaluación de tormentas y de danas) para que pueda 
ser  analizado por dagda_agent y generar previsiones de alertas y trabajos a realizar.

dagda_agent.py: Examina el último fichero de log que contiene los análisis diarios generados por dagda (tiempo actual, previsión a tres días, previsión de tormentas y  
previsión de danas) para generar un prompt de Gemini y obtener una descripción detallada de las alertas meteorológicas y las acciones que se deberían ejecutar. 
Genera un fichero con la descripción y envía un correo.

Requisitos: 

Sistema operativo: Linux (Debian, Ubuntu o similar)
Intérprete: python3
Librerías: Google AI Studio y dotenv para Python.
En Debian/Ubuntu puedes instalar las dependencias necesarias con: 
   $ pip install google-genai
   $ pip install python-dotenv
A partir de Debian 12, Python protege el entorno global del sistema mediante la directiva EXTERNALLY-MANAGED (PEP 668), por lo que la forma correcta y recomendada de instalar 
librerías de Python es mediante un entorno virtual (venv). Si no quieres complicarte la vida con entornos virtuales puedes usar la opción --break-system-packages de pip.
   $ pip install google-genai --break-system-packages
   $ pip install python-dotenv --break-system-packages

Es requisito disponer de usuario en Google AI Studio y generar una API KEY.
Para poder ejecutar este módulo es necesario tener instalado también el módulo Dagda Analysis

Instalación:

Descargar los ficheros run_dagda.sh y dagda_agent.py

En el mismo directorio donde se va a ejecutar dagda_agent.py crear un fichero .env con las variables de entorno necesarias:
  GCP_API_KEY="TU_API_KEY"
  ARO_EMAIL_REMITENTE="CORREO DEL REMITENTE"
  ARO_EMAIL_PASSWORD="PASSWORD O TOKEN DEL REMITENTE"
  ARO_EMAIL_DESTINATARIO="CORREO DEL DESTINATARIO"

Dar permisos de ejecución a run_dagda.sh
   $ chmod +x /tu_directorio/run_dagda.sh 

Ejecución:

Tenemos dos formas de ejecutar Dagda Agent, de manera puntual o periódicamente mediante el cron del sistema

De manera puntual:

Generar el fichero de log con los análisis:
   $ ./run_dagda.sh
Generar el boletín meteorológico:
   $ python3 dagda_agent.py

Añadirlo al cron:
	$ crontab -e
	Añadir al final del fichero:
		0 8 * * * /tu_directorio/run_dagda.sh
		30 8 * * * /usr/bin/python3 /tu_directorio/dagda_agent.py >> /tu_directorio/dagda_agent.log 2>&1
	Salvar y salir

De esta manera run_dagda.sh se ejecutará todos los días a las 8 AM y dagda_agent.py a las 8:30 AM.


2. VIVERO (En pensamiento…)
Módulo pensado para calcular el riego necesario basándose en la evapotranspiración de referencia y determinar el agua exacta que 
necesita un cultivo, identificar los mejores días para realizar el abonado dependiendo de las condiciones climáticas y conocer el 
estado del suelo de cultivo en general.
