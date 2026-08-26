# -----------------------------------------------------------------------------------------------
# Módulo: dagda_agent
# Descripción: Examina el último fichero de log que contiene los análisis diarios generados
# por dagda (tiempo actual, previsión a tres días, previsión de tormentas y  previsión de
# danas) para generar un prompt de Gemini y obtener una descripción detallada de las alertas
# meteorológicas y las acciones que se deberían ejecutar. Genera un fichero con la descripción
# y envía un correo.
# -----------------------------------------------------------------------------------------------

#!/usr/bin/env python3
import os
import glob
import smtplib
from datetime import datetime
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText 
from google import genai
from dotenv import load_dotenv

# ============================================================
# CONFIGURACIÓN
# ============================================================

# Carga las variables desde el archivo .env de la carpeta local
load_dotenv()

# Lee la clave desde las variables de entorno del sistema:
GEMINI_API_KEY = os.environ.get("GCP_API_KEY")

# Configuración de correo (parte desde las variables de entorno)
ENVIAR_CORREO = True  # Ponlo en False si quieres desactivar el envío temporalmente
EMAIL_REMITENTE = os.environ.get("ARO_EMAIL_REMITENTE")
EMAIL_PASSWORD = os.environ.get("ARO_EMAIL_PASSWORD")
EMAIL_DESTINATARIO = os.environ.get("ARO_EMAIL_DESTINATARIO")
SERVIDOR_SMTP = "smtp.gmail.com"
PUERTO_SMTP = 587

# Directorios de logs de entrada y de salida de previsiones
log_dir = os.path.expanduser("~/Programacion/ARO/logs")
previsiones_dir= os.path.expanduser("~/Programacion/ARO/previsiones")


# =====================================================================
# 1. OBTENER EL ÚLTIMO ARCHIVO DE LOG GENERADO
# =====================================================================

list_of_files = glob.glob(f"{log_dir}/*.log")

if not list_of_files:
    print("No se encontraron archivos de log.")
    exit(1)

latest_file = max(list_of_files, key=os.path.getctime)

with open(latest_file, "r") as f:
    raw_dagda_output = f.read()

# =====================================================================
# 2. CONSULTAR A LA API DE GEMINI
# =====================================================================

# Inicializamos el cliente pasando la API Key directamente
client = genai.Client(api_key=GEMINI_API_KEY)

# Prompt con la plantilla que acordamos
prompt = f"""
Eres Dagda-Agent, el asistente meteorológico personal para la zona de Cabida. Tu objetivo es interpretar la salida técnica de 'dagda' y traducirla en impactos operacionales claros para la planificación agrícola y de riesgo.

A continuación tienes la salida en texto plano generada por el módulo 'dagda':

--- INICIO DATOS DAGDA ---
{raw_dagda_output}
--- FIN DATOS DAGDA ---

Genera un informe de situacion estructurado utilizando ESTRICTAMENTE el siguiente formato:

**Asunto:** Previsión Dagda para Cabida ([Rango de fechas]): Resumen operativo e impactos

**Resumen Ejecutivo:**
[Breve frase con el resumen general del periodo: p. ej., "Semana de alta variabilidad con ventana crítica de tormentas el 20 y riesgo de DANA hacia el 26."]

**Desglose por episodios:**

* **[Fecha 1] — [Titular del fenómeno]**
* **Escenario y riesgos:** [Descripción del tiempo en superficie, horarios probables, riesgo de granizo, reventones o incendios].
* **Soporte técnico:** [Métricas clave: T500, Z500, LI, CAPE, RH700, Delta Z500, etc.].

(Repite este bloque para cada fecha relevante con inestabilidad, cambios de masa de aire o DANA)

**Recomendaciones operativas:**
* [1-2 puntos breves con medidas preventivas sugeridas según los datos: p. ej., revisar drenajes/riego antes del 20, precaución con trabajos de campo por incendios el 19, etc.]
"""

# Iniciar la consulta al modelo
chat = client.chats.create(model="gemini-3.6-flash")
response = chat.send_message(prompt)
texto_resultado = response.text

# =====================================================================
# 3. MOSTRAR EN PANTALLA Y GUARDAR EN ARCHIVO
# =====================================================================
# Imprimir por consola
print(response.text)

# Crear directorio de previsiones si no existe
os.makedirs(previsiones_dir, exist_ok=True)

# Generar el nombre de archivo con la fecha/hora actual
fecha_actual = datetime.now().strftime("%Y-%m-%d")
archivo_salida = os.path.join(previsiones_dir, f"prevision_{fecha_actual}.txt")


# Escribir la previsión en el archivo
with open(archivo_salida, "w", encoding="utf-8") as f:
    f.write(texto_resultado)

print(f"\n[+] Previsión guardada con éxito en: {archivo_salida}")


# =====================================================================
# 4. ENVIAR POR CORREO ELECTRÓNICO
# =====================================================================
if ENVIAR_CORREO:
    try:
        msg = MIMEMultipart()
        msg['From'] = EMAIL_REMITENTE
        msg['To'] = EMAIL_DESTINATARIO
        msg['Subject'] = f"Previsión Dagda Cabida ({datetime.now().strftime('%d/%m/%Y')})"

    # Adjuntamos el texto del boletín al cuerpo del correo
        msg.attach(MIMEText(texto_resultado, 'plain', 'utf-8'))

        with smtplib.SMTP(SERVIDOR_SMTP, PUERTO_SMTP) as server:
            server.starttls()
            server.login(EMAIL_REMITENTE, EMAIL_PASSWORD)
            server.send_message(msg)
            
        print(f"[+] Correo enviado con éxito a {EMAIL_DESTINATARIO}")
    except Exception as e:
        print(f"[-] Error al enviar el correo: {e}")
        
