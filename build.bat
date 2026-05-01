@echo off
setlocal

set "BUILD_DIR=build"
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

pushd "%BUILD_DIR%"
wcl386 -cc -bt=nt -fe=mode_table_test.exe ..\tools\mode_table_test.c ..\src\igd9x.c ..\src\igd9x_mmio.c ..\src\igd9x_pci.c ..\src\igd9x_modes.c ..\src\igd9x_driver.c ..\src\igd9x_hw_stub.c ..\src\intel\igd9x_intel_backend.c ..\src\win9x\miniport.c ..\src\win9x\vxd.c ..\src\win9x\igd9x_win9x_mmio.c ..\src\win9x\igd9x_win9x_pci.c ..\src\win9x\igd9x_win9x_entry.c ..\src\win9x\igd9x_win9x_exports.c ..\src\win9x\igd9x_win9x_driver.c -I..\include
popd
