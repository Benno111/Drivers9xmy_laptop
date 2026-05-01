param(
    [string]$RepoRoot = (Split-Path -Parent (Split-Path -Parent $PSScriptRoot)),
    [string]$BuildRoot = (Join-Path (Split-Path -Parent (Split-Path -Parent $PSScriptRoot)) "build"),
    [string]$ReleaseRoot = (Join-Path (Split-Path -Parent (Split-Path -Parent $PSScriptRoot)) "release\win9x")
)

$ErrorActionPreference = "Stop"

$packageRoot = $PSScriptRoot
$releaseFiles = @(
    "igd9x.drv",
    "igd9x.vxd",
    "package\win9x\igd9x.inf",
    "package\win9x\README.md",
    "package\win9x\exports.def"
)

& (Join-Path $packageRoot "build-driver.ps1") -RepoRoot $RepoRoot -BuildRoot $BuildRoot

New-Item -ItemType Directory -Force -Path $ReleaseRoot | Out-Null

$released = @()
foreach ($entry in $releaseFiles) {
    $sourcePath = Join-Path $RepoRoot $entry
    if ($entry -eq "igd9x.drv") {
        $sourcePath = Join-Path $BuildRoot "igd9x.drv"
    } elseif ($entry -eq "igd9x.vxd") {
        $sourcePath = Join-Path $BuildRoot "igd9x.vxd"
    }

    if (-not (Test-Path $sourcePath)) {
        Write-Host "Skipping missing release file: $entry"
        continue
    }

    $targetPath = Join-Path $ReleaseRoot (Split-Path $entry -Leaf)
    Copy-Item -LiteralPath $sourcePath -Destination $targetPath -Force
    $released += (Split-Path $entry -Leaf)
}

$manifestPath = Join-Path $ReleaseRoot "release-manifest.txt"
$manifestLines = @()
$manifestLines += "IGD9X Win9x release manifest"
$manifestLines += "RepoRoot=$RepoRoot"
$manifestLines += "BuildRoot=$BuildRoot"
$manifestLines += "ReleaseRoot=$ReleaseRoot"
$manifestLines += ""
$manifestLines += "Final files:"
foreach ($file in ($released | Sort-Object -Unique)) {
    $manifestLines += "  $file"
}

$manifestLines | Set-Content -LiteralPath $manifestPath -Encoding ASCII
Write-Host "Wrote release files to $ReleaseRoot"
Write-Host "Wrote manifest to $manifestPath"
