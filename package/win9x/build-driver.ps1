param(
    [string]$RepoRoot = (Split-Path -Parent (Split-Path -Parent $PSScriptRoot)),
    [string]$BuildRoot = (Join-Path (Split-Path -Parent (Split-Path -Parent $PSScriptRoot)) "build")
)

$ErrorActionPreference = "Stop"

$packageRoot = $PSScriptRoot
$linkerFile = Join-Path $BuildRoot "driver.lnk"
$vxdLinkerFile = Join-Path $BuildRoot "vxd.lnk"
$sourceLinker = Join-Path $packageRoot "driver.lnk"
$sourceVxdLinker = Join-Path $packageRoot "vxd.lnk"
$watcomLibNt = Join-Path $env:WATCOM "lib386\nt"
$watcomLibRoot = Join-Path $env:WATCOM "lib386"

if (-not (Test-Path $BuildRoot)) {
    New-Item -ItemType Directory -Force -Path $BuildRoot | Out-Null
}

& (Join-Path $RepoRoot "build.bat")

Copy-Item -LiteralPath $sourceLinker -Destination $linkerFile -Force
Copy-Item -LiteralPath $sourceVxdLinker -Destination $vxdLinkerFile -Force

$originalLib = $env:LIB
$env:LIB = "$watcomLibNt;$watcomLibRoot"

Push-Location $BuildRoot
try {
    & wasm ..\src\win9x\igd9x_win9x_vxd_chkstub.asm
    if ($LASTEXITCODE -ne 0) {
        throw "wasm failed while building igd9x_win9x_vxd_chkstub.obj with exit code $LASTEXITCODE"
    }

    & wlink "@driver.lnk"
    if ($LASTEXITCODE -ne 0) {
        throw "wlink failed while building igd9x.dll with exit code $LASTEXITCODE"
    }

    & wlink "@vxd.lnk"
    if ($LASTEXITCODE -ne 0) {
        throw "wlink failed while building igd9x.vxd with exit code $LASTEXITCODE"
    }
}
finally {
    Pop-Location
    $env:LIB = $originalLib
}

Copy-Item -LiteralPath (Join-Path $BuildRoot "igd9x.dll") -Destination (Join-Path $BuildRoot "igd9x.drv") -Force

Write-Host "Linked Win9x driver: $(Join-Path $BuildRoot 'igd9x.dll')"
Write-Host "Linked Win9x VxD: $(Join-Path $BuildRoot 'igd9x.vxd')"
