#!/usr/bin/env bash
set -euo pipefail

ensure_idf() {
  if command -v idf.py >/dev/null 2>&1; then
    return 0
  fi

  local default_export="$HOME/esp/esp-idf/export.sh"
  if [[ -f "${default_export}" ]]; then
    # shellcheck disable=SC1090
    source "${default_export}" >/dev/null 2>&1 || true
  fi

  if ! command -v idf.py >/dev/null 2>&1; then
    echo "ERROR: idf.py not found. Install ESP-IDF and source export.sh" >&2
    echo "Try: source ~/esp/esp-idf/export.sh" >&2
    exit 2
  fi
}
