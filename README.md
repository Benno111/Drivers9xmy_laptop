# IGD9X

Win9x-oriented display-driver scaffold for Intel HD Graphics 4000-class hardware.

This repository does **not** yet contain a finished, loadable Windows 9x display
driver. What it does provide is a clean starting point for the parts we can
build safely here:

- a mode database with common "proper" desktop timings
- mode validation and pitch calculation
- a Win9x-style driver facade that separates OS glue from hardware ops
- PCI device discovery and exact-family filtering for Intel Ivy Bridge GT1/GT2 devices
- a hardware-abstraction layer stub for later Intel register work
- a small host-side test utility to verify mode enumeration

## Scope

Intel HD Graphics 4000 is a modern integrated GPU family that was never shipped
with native Windows 9x support. A real port needs low-level hardware bring-up
that depends on the exact PCI device, memory map, display pipes, PLLs, and
panel routing of the target machine.

This scaffold is intended to be the foundation for that work:

- enumerate modes we want the driver to expose
- keep the mode logic separate from hardware programming
- model the miniport/display split that a Win9x port needs
- make it easy to swap in an Intel-specific backend later

## Current mode set

The default catalog covers the legacy modes most people expect from a Win9x
desktop driver:

- 640x480
- 800x600
- 1024x768
- 1280x720
- 1280x800
- 1366x768
- 1400x1050
- 1440x900
- 1600x900
- 1600x1200
- 1920x1080

Each resolution is provided with 16-bit and 32-bit variants where that makes
sense for the target VRAM and pixel-clock envelope.

## What is missing

To turn this into a real Win9x Intel driver, we still need:

- MMIO register definitions for the HD 4000 display engine
- mode programming for pipes, planes, cursor, and PLLs
- BIOS/VBE fallback path for boards where firmware exposes usable modes
- Win9x DDK integration and INF packaging

## Files

- `include/igd9x.h` - shared types and public API for the mode layer
- `include/igd9x_driver.h` - Win9x-oriented driver facade and hardware ops
- `src/igd9x.c` - mode validation and lookup helpers
- `src/igd9x_modes.c` - the built-in mode table
- `src/igd9x_driver.c` - driver attach, mode enumeration, and mode switching
- `src/igd9x_hw_stub.c` - placeholder for Intel-specific hardware access
- `include/igd9x_intel.h` - Intel PCI, MMIO, and timing-plan definitions
- `src/intel/igd9x_intel_backend.c` - Intel backend skeleton and register plan builder
- `include/igd9x_mmio.h` - shared MMIO read/write shim interface
- `include/igd9x_pci.h` - PCI discovery and config-space helpers
- `include/igd9x_win9x_mmio.h` - Win9x MMIO binding API used by the shim
- `include/igd9x_win9x_pci.h` - Win9x PCI config-space mock used by the test harness
- `src/igd9x_mmio.c` - MMIO helper wrappers
- `src/igd9x_pci.c` - PCI discovery and BAR sizing logic
- `src/win9x/igd9x_win9x_mmio.c` - Win9x host-side MMIO region mock
- `src/win9x/igd9x_win9x_pci.c` - Win9x host-side PCI config-space mock
- `include/igd9x_win9x_entry.h` - Win9x entry/context lifecycle
- `src/win9x/igd9x_win9x_entry.c` - Win9x discovery, attach, and mode handoff
- `include/igd9x_win9x_exports.h` - exported Win9x driver entrypoint API
- `src/win9x/igd9x_win9x_exports.c` - exported Win9x lifecycle wrappers
- `include/igd9x_win9x_driver.h` - Win9x loader-facing `DriverEntry` declaration
- `src/win9x/igd9x_win9x_driver.c` - minimal Win9x driver entry shim
- `src/win9x/igd9x_win9x_module.def` - module export definition map
- `src/win9x/miniport.c` - Win9x-facing miniport wrapper
- `src/win9x/vxd.c` - Win9x low-level hardware service hook
- `package/win9x/` - packaging skeleton with INF and staging manifests
- `docs/win9x-layout.md` - architecture notes for the Win9x split
- `tools/mode_table_test.c` - host-side verification program

## Build

If you have Open Watcom installed, the included batch file will build the
verification program:

```bat
build.bat
```

The equivalent compiler invocation is:

```bat
wcl386 -cc -bt=nt -fe=mode_table_test.exe tools\mode_table_test.c src\igd9x.c src\igd9x_mmio.c src\igd9x_pci.c src\igd9x_modes.c src\igd9x_driver.c src\igd9x_hw_stub.c src\intel\igd9x_intel_backend.c src\win9x\miniport.c src\win9x\vxd.c src\win9x\igd9x_win9x_mmio.c src\win9x\igd9x_win9x_pci.c src\win9x\igd9x_win9x_entry.c src\win9x\igd9x_win9x_exports.c src\win9x\igd9x_win9x_driver.c -Iinclude
```

Run the resulting executable to print the supported mode table:

```bat
mode_table_test.exe
```

## Win9x Package

The `package/win9x/` directory contains the shipping skeleton for the Win9x
driver package:

- `package/win9x/igd9x.inf` - install template
- `package/win9x/exports.def` - package export map
- `package/win9x/driver.lnk` - Watcom linker response file for the driver DLL
- `package/win9x/build-driver.ps1` - script that links the Win9x driver binary
- `package/win9x/release.ps1` - script that gathers the final files into `release/win9x/`
- `package/win9x/files.staging.txt` - staged payload manifest
- `package/win9x/driverentry.c` - package-side loader entry wrapper
- `package/win9x/stage.ps1` - staging script that assembles a release payload

To build the package layout into `dist/win9x/`, run:

```powershell
powershell -ExecutionPolicy Bypass -File .\package\win9x\stage.ps1
```

That produces a payload folder plus a staging manifest listing the files that
were copied.

To link the Win9x driver DLL itself, run:

```powershell
powershell -ExecutionPolicy Bypass -File .\package\win9x\build-driver.ps1
```

To gather the final release files into `release/win9x/`, run:

```powershell
powershell -ExecutionPolicy Bypass -File .\package\win9x\release.ps1
```
