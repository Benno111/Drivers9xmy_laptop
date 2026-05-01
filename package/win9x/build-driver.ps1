param(
    [string]$RepoRoot = (Split-Path -Parent (Split-Path -Parent $PSScriptRoot)),
    [string]$BuildRoot = (Join-Path (Split-Path -Parent (Split-Path -Parent $PSScriptRoot)) "build")
)

$ErrorActionPreference = "Stop"

$packageRoot = $PSScriptRoot
$linkerFile = Join-Path $BuildRoot "driver.lnk"
$sourceLinker = Join-Path $packageRoot "driver.lnk"

if (-not (Test-Path $BuildRoot)) {
    New-Item -ItemType Directory -Force -Path $BuildRoot | Out-Null
}

& (Join-Path $RepoRoot "build.bat")

Copy-Item -LiteralPath $sourceLinker -Destination $linkerFile -Force

Push-Location $BuildRoot
try {
    & wlink "@driver.lnk"
}
finally {
    Pop-Location
}

Copy-Item -LiteralPath (Join-Path $BuildRoot "igd9x.dll") -Destination (Join-Path $BuildRoot "igd9x.vxd") -Force
Copy-Item -LiteralPath (Join-Path $BuildRoot "igd9x.dll") -Destination (Join-Path $BuildRoot "igd9x.drv") -Force

Write-Host "Linked Win9x driver: $(Join-Path $BuildRoot 'igd9x.dll')"
