# Win9x Package Layout

This directory holds the shipping layout for the Win9x build of IGD9X.

It is intentionally a package skeleton rather than a finished installer.
The goal is to show how the entrypoint, export map, and driver payload would
be assembled for a real Windows 9x build.

## Contents

- `igd9x.inf` - installation template for the Win9x driver package
- `files.staging.txt` - files that should be copied into the release payload
- `exports.def` - package-level export definition map
- `driver.lnk` - Watcom linker response file for the Win9x driver DLL
- `vxd.lnk` - Watcom linker response file for the Win9x VxD binary
- `build-driver.ps1` - PowerShell script that links the DLL and VxD binaries
- `release.ps1` - PowerShell script that gathers the final release files
- `driverentry.c` - Win9x loader-facing entry wrapper for the packaged build
- `stage.ps1` - PowerShell staging script that builds the release payload layout

## What a real build would do

1. Compile the Win9x driver sources.
2. Link them using the package export map.
3. Copy the resulting binary and INF into a staging directory.
4. Install the package using the hardware ID section that matches the target
   Intel device.

The `release.ps1` script builds on that flow by collecting the final driver
binary, linked `igd9x.vxd`, INF, and packaging notes into `release/win9x/`.
