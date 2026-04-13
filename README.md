# ESP32C6 PC Remote Power Switch

Remote PC power control on XIAO ESP32C6.

Current implementation phase:

- Safe hardware pulse engine for PWR_SW and RESET_SW
- Power-state sensing via PWR_LED input
- Command state machine: power_on, power_off, reboot_soft, reboot_hard
- UART local command console for bench tests
- Build profiles: matter (primary), zigbee (fallback)
- Matter onboarding QR generator from MT payload

## Repository layout

- main/: firmware sources (ESP-IDF)
- scripts/: build and flash helpers
- tools/: payload and QR utilities
- docs/: wiring, BOM, and installation notes

## Quick start (macOS)

1. Install ESP-IDF and export environment in your shell session:

   source ~/esp/esp-idf/export.sh

   Note: build/flash/monitor scripts also try to auto-source this path.

2. Prepare Python tools environment:

   python3 -m venv .venv
   ./.venv/bin/pip install -r tools/requirements.txt

   or run:

   ./scripts/setup_python_tools.sh

3. Build Matter profile:

   ./scripts/build_matter.sh

4. Flash Matter profile:

   ./scripts/flash_matter.sh /dev/cu.usbmodemXXXX
   or with explicit baud:
   ./scripts/flash_matter.sh /dev/cu.usbmodemXXXX 115200

5. Monitor logs:

   ./scripts/monitor.sh /dev/cu.usbmodemXXXX

6. In serial monitor, test commands:

   help
   state
   on
   off
   reboot_soft
   reboot_hard

Before build/flash, you can validate environment:

./scripts/check_env.sh

## Build Zigbee fallback profile

./scripts/build_zigbee.sh
./scripts/flash_zigbee.sh /dev/cu.usbmodemXXXX
./scripts/flash_zigbee.sh /dev/cu.usbmodemXXXX 115200

## Matter onboarding QR generator

If firmware logs print an onboarding payload like MT:XXXX, generate QR code:

./.venv/bin/python tools/generate_matter_qr.py --payload MT:XXXX --png out/matter_qr.png --svg out/matter_qr.svg

If payload is inside a monitor log file:

./.venv/bin/python tools/extract_matter_payload.py --log monitor.log

## Wiring and diagrams

- Breadboard test: docs/breadboard_test.md
- Final in-PC installation: docs/final_pc_install.md
- BOM: docs/bom.md

## Important safety notes

- Use optocouplers on motherboard switch and LED header lines.
- Keep manual case button connected in parallel with ESP outputs.
- Never connect raw motherboard header lines directly to ESP GPIO.
- reboot_hard is emergency-only. Prefer reboot_soft.

## Next implementation milestone

- Replace protocol placeholder in main/protocol_bridge.c with real esp-matter endpoint handlers.
- Add Zigbee endpoint handlers with equivalent command mapping.
