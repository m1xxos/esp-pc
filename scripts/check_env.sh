#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
# shellcheck disable=SC1091
source "${SCRIPT_DIR}/lib_idf.sh"

EXIT_CODE=0

echo "[check] project: ${ROOT_DIR}"

if command -v idf.py >/dev/null 2>&1; then
  echo "[ok] idf.py: $(command -v idf.py)"
  idf.py --version || true
else
  default_export="$HOME/esp/esp-idf/export.sh"
  if [[ -f "${default_export}" ]]; then
    echo "[warn] idf.py not in PATH, but found ${default_export}"
    echo "       Run: source ${default_export}"
  else
    echo "[error] idf.py not found in PATH"
    echo "        Install ESP-IDF and source export script in this shell"
  fi
  EXIT_CODE=2
fi

PYTHON_BIN="${ROOT_DIR}/.venv/bin/python"
PIP_BIN="${ROOT_DIR}/.venv/bin/pip"

if [[ ! -x "${PYTHON_BIN}" ]]; then
  echo "[warn] Python virtualenv not found at ${ROOT_DIR}/.venv"
  echo "       Create it with: python3 -m venv .venv"
  echo "       Then install tools: ./.venv/bin/pip install -r tools/requirements.txt"
else
  echo "[ok] python: ${PYTHON_BIN}"
  "${PYTHON_BIN}" --version
  if "${PYTHON_BIN}" -c "import qrcode" >/dev/null 2>&1; then
    echo "[ok] qrcode dependency installed"
  else
    echo "[warn] qrcode dependency missing"
    echo "       Install with: ${PIP_BIN} install -r tools/requirements.txt"
  fi
fi

echo "[hint] available serial ports:"
ls /dev/cu.usb* /dev/tty.usb* 2>/dev/null || echo "  (none detected)"

echo "[done] environment check complete"
exit ${EXIT_CODE}
