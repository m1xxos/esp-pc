#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
# shellcheck disable=SC1091
source "${SCRIPT_DIR}/lib_idf.sh"
ensure_idf

idf.py \
  -D IDF_TARGET=esp32c6 \
  -D APP_PROTOCOL=zigbee \
  -D SDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.defaults.zigbee" \
  build
