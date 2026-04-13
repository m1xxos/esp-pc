# Breadboard Test Layout

This layout validates command pulses and state sensing before installing inside the PC.

## Test goal

- Validate GPIO pulse timing for PWR_SW and RESET_SW control lines.
- Validate power-state sensing via PWR_LED line through an isolator.
- Confirm cooldown and command sequencing behavior.

## Wiring map

- XIAO GPIO1 (D1) -> 220R -> Optocoupler U1 input LED (PWR_SW command)
- XIAO GPIO2 (D2) -> 220R -> Optocoupler U2 input LED (RESET_SW command)
- XIAO GPIO0 (D0) <- Optocoupler U3 output transistor (PWR_LED sense)
- XIAO GND -> Breadboard logic GND only
- Do not short motherboard-side switch lines to ESP GND directly

## Bench harness

Use a mock harness first:

- 5V pull-up resistor (10k) emulating motherboard switch detect
- Push button or jumper emulating switch closure path
- LED + resistor showing ON/OFF state line for U3 sensing tests

After mock harness works, connect to actual motherboard front-panel headers.

## Diagram source

See diagram at docs/diagrams/breadboard_test.mmd

## Validation checklist

1. on command emits short pulse (about 120 ms)
2. off command emits long pulse (about 6000 ms)
3. reboot_soft performs off then on sequence
4. reboot_hard emits RESET pulse only
5. state command tracks PWR_LED transitions
