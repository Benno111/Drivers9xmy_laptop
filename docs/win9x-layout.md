# Win9x Layout

This repository now separates the display-driver work into three layers:

## Core mode layer

The files under `src/igd9x*.c` own the mode table, validation, and mode
selection logic. This is hardware-neutral and can be reused by both the display
front end and the hardware backend.

## Miniport front end

`src/win9x/miniport.c` is the Win9x-facing wrapper. In a real DDK build this is
where the OS entry points would live. For now it forwards to the shared driver
facade so the attach/enumerate/set flow is testable on the host.

The lifecycle glue now lives in `src/win9x/igd9x_win9x_entry.c`. That module is
the closest thing to the real driver bring-up path in this repository: it owns
PCI discovery, MMIO binding, backend import, driver attach, mode requests, and
shutdown.

`src/win9x/igd9x_win9x_exports.c` wraps that lifecycle in exported functions
that a real Win9x driver package can surface to the OS. The current exports are
still host-build friendly, but they give us the shape of the eventual DDK entry
surface.

`src/win9x/igd9x_win9x_driver.c` adds the loader-facing `DriverEntry` shim, and
`src/win9x/igd9x_win9x_module.def` records the export set for a real package.

The package skeleton under `package/win9x/` shows how these pieces would ship:
an INF template, an export map, and a staging manifest for the build output.
The package also includes `stage.ps1`, which assembles a release payload folder
from the source tree and package files.
`package/win9x/driver.lnk` now drives the Watcom link step for `igd9x.dll`, and
`package/win9x/build-driver.ps1` wraps that linker response file in a simple
package-side build command.
`package/win9x/release.ps1` collects the final deliverables into
`release/win9x/` once the driver has been linked.

## VxD / hardware backend

`src/win9x/vxd.c` represents the low-level Win9x hardware service layer. For
now it exposes the stub backend. On actual hardware this layer would own:

- PCI discovery and filter decisions
- MMIO mapping
- pipe/PLL/plane programming
- BIOS or VBE fallback setup

The PCI discovery shim lives in `src/win9x/igd9x_win9x_pci.c`. In the host
build it presents a mock PCI function so we can exercise the scanner and BAR0
sizing logic. In a real Win9x port this layer would translate the config-space
accesses to the system's PCI services.

The Win9x MMIO shim lives in `src/win9x/igd9x_win9x_mmio.c`. In the host
build it uses a memory-backed region so we can validate register writes. In a
real Win9x port this is the layer that would map the Intel MMIO aperture and
forward `write32`/`read32` calls into the mapped BAR.

The Intel-specific register definitions and register-write plan builder now live
in `include/igd9x_intel.h` and `src/intel/igd9x_intel_backend.c`. That backend
is still a skeleton, but it already knows how to identify Ivy Bridge GT1 and
GT2 PCI IDs and build a legacy pipe timing plan for the common desktop modes.

## What remains for a real Intel HD 4000 port

- identify the exact GT/Ivy Bridge display registers needed for the laptop
- replace the plan builder with real MMIO writes in the Win9x driver path
- wire the wrappers into the Win9x DDK build and INF packaging
