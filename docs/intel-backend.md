# Intel Backend

This repository now has an Intel-specific backend skeleton for the Ivy Bridge
class of integrated graphics controllers commonly branded as Intel HD 4000.

## What it covers

- PCI discovery for Intel display controllers
- chipset identification for Ivy Bridge GT1 and GT2 parts
- BAR0 sizing from PCI config space
- legacy MMIO register names and offsets for pipe, plane, and DPLL setup
- a register-write plan builder for a legacy pipe modeset
- a bindable MMIO shim so plan execution can be routed to real hardware later

## What it does not do yet

- it does not map real hardware MMIO in a Win9x driver
- it does not yet own the Win9x MMIO mapping call
- it does not yet perform a real Win9x PCI bus scan
- it does not program the actual Intel display engine registers
- it does not yet parse VBT, panel data, or BIOS-provided backlight routing

## Why this layer exists

The goal is to keep the mode logic and the hardware programming separate. That
lets us verify timings and mode selection on the host while the real Intel
register work gets filled in later.

## Sources used for the register layout

- Intel open-source Ivy Bridge PRM PDFs hosted by x.org
- the IntelRegisters reference on the DRI wiki
- X.Org chipset mappings for Ivy Bridge device IDs such as `0x0162` and `0x0166`
