# Matter and Yandex Integration Notes

Target mode:

- Primary profile: Matter
- Backup profile: Zigbee

Reason for backup profile:

- Controller ecosystems can differ in Matter device support and onboarding behavior.
- Zigbee profile keeps the same power-control semantics for fast fallback.

Current firmware state:

- Protocol bridge is implemented as placeholder transport in main/protocol_bridge.c
- Command mapping to GPIO state machine is already functional

Next protocol work:

- Add real esp-matter endpoint callbacks to invoke protocol_bridge_invoke_command
- Add equivalent Zigbee endpoint callbacks
