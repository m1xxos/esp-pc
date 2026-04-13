#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
# shellcheck disable=SC1091
source "${SCRIPT_DIR}/lib_idf.sh"
ensure_idf

PORT="${1:-}"
BAUD="${2:-115200}"

if [[ -z "${PORT}" ]]; then
  echo "Usage: $0 <serial_port> [baud]"
  echo "Example: $0 /dev/cu.usbmodem101 115200"
  exit 1
fi

idf.py \
  -D IDF_TARGET=esp32c6 \
  -D APP_PROTOCOL=matter \
  -D SDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.defaults.matter" \
  -p "${PORT}" \
  -b "${BAUD}" \
  flash
