# Final PC Installation Layout

Install inside the PC case after breadboard tests pass.

## Installation goals

- Keep manual case buttons fully functional.
- Add ESP-driven parallel control for PWR_SW and RESET_SW.
- Sense power state from PWR_LED lines.

## Required topology

- Case PWR button remains connected to motherboard PWR_SW.
- ESP optocoupler output for PWR command is connected in parallel to the same PWR_SW pins.
- Case RESET button remains connected to motherboard RESET_SW.
- ESP optocoupler output for hard reboot is connected in parallel to RESET_SW pins.
- PWR_LED is sensed through an optocoupler input path.

## Wiring map

- U1 output transistor across motherboard PWR_SW pins (parallel with case power button)
- U2 output transistor across motherboard RESET_SW pins (parallel with case reset button)
- U3 input LED across motherboard PWR_LED+ and PWR_LED- with current limit resistor
- U3 transistor output to XIAO GPIO0 sense input

## Mechanical notes

- Mount the XIAO and optocoupler board using non-conductive standoffs.
- Keep wire runs short and twisted where possible.
- Route low-voltage logic wiring away from PSU high-current cables.

## Diagram source

See diagram at docs/diagrams/final_pc_install.mmd

## Power-up test sequence

1. Verify manual case power button works.
2. Verify manual case reset button works.
3. Run UART command on and check boot.
4. Run UART command off and verify shutdown.
5. Run reboot_soft and reboot_hard separately.
