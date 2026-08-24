# -----------------------------------------------------------------------------
# Módulo: run_dagda
# Descripción: Genera un fichero de log con la salida del análisis de Dagda
# (tiempo actual, previsión a tres días, evaluación de tormentas y de danas)
# para que pueda ser  analizado por dagda_agent y generar previsiones de alertas
# y trabajos a realizar.
# -----------------------------------------------------------------------------

#!/usr/bin/env bash
set -euo pipefail

DAGDA_DIR="$HOME/Programacion/ARO/"
DAGDA_BIN="$DAGDA_DIR/dagda"
LOG_DIR="$DAGDA_DIR/logs"
OUT_FILE="$LOG_DIR/completo_$(date +%Y-%m-%d).log"

# Crear directorio de logs si no existe
#mkdir -p "$LOG_DIR"

# Crea el fichero diario con la primera cabecera y luego Añade el resto de información al final del mismo fichero
echo -e "\n--- TIEMPO ACTUAL (-a) ---" > "$OUT_FILE"
"$DAGDA_BIN" -a >> "$OUT_FILE"

echo -e "\n--- PREVISION 3 DIAS (-p) ---" >> "$OUT_FILE"
"$DAGDA_BIN" -p >> "$OUT_FILE"

echo -e "\n--- EVALUACION TORMENTAS 2 DIAS (-t) ---" >> "$OUT_FILE"
"$DAGDA_BIN" -t >> "$OUT_FILE"

echo -e "\n--- EVALUACION DANAS 14 DIAS (-d) ---" >> "$OUT_FILE"
"$DAGDA_BIN" -d >> "$OUT_FILE"
