#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
# shellcheck disable=SC1091
source "${SCRIPT_DIR}/lib_idf.sh"
ensure_idf

PORT="${1:-}"

if [[ -z "${PORT}" ]]; then
  echo "Usage: $0 <serial_port>"
  echo "Example: $0 /dev/cu.usbmodem101"
  exit 1
fi

idf.py -p "${PORT}" monitor
