param(
    [string]$RepoRoot = (Split-Path -Parent (Split-Path -Parent $PSScriptRoot)),
    [string]$OutputRoot = (Join-Path (Split-Path -Parent (Split-Path -Parent $PSScriptRoot)) "dist\win9x")
)

$ErrorActionPreference = "Stop"

$packageRoot = $PSScriptRoot
$payloadRoot = Join-Path $OutputRoot "payload"
$manifestPath = Join-Path $OutputRoot "staging-manifest.txt"
$stagedFiles = @()

New-Item -ItemType Directory -Force -Path $payloadRoot | Out-Null

$manifest = Join-Path $packageRoot "files.staging.txt"
Get-Content $manifest | ForEach-Object {
    $line = $_.Trim()
    if ([string]::IsNullOrWhiteSpace($line)) {
        return
    }
    if ($line.StartsWith(";")) {
        return
    }

    $sourcePath = Join-Path $RepoRoot $line
    if (-not (Test-Path $sourcePath)) {
        Write-Host "Skipping missing file: $line"
        return
    }

    $relativeTarget = $line.Replace("/", "\")
    $targetPath = Join-Path $payloadRoot $relativeTarget
    $targetDir = Split-Path $targetPath -Parent

    New-Item -ItemType Directory -Force -Path $targetDir | Out-Null
    Copy-Item -LiteralPath $sourcePath -Destination $targetPath -Force
    $stagedFiles += $relativeTarget
}

$packageFiles = @(
    "package\win9x\igd9x.inf",
    "package\win9x\exports.def",
    "package\win9x\driver.lnk",
    "package\win9x\build-driver.ps1",
    "package\win9x\README.md",
    "package\win9x\driverentry.c",
    "package\win9x\stage.ps1"
)

foreach ($entry in $packageFiles) {
    $sourcePath = Join-Path $RepoRoot $entry
    if (-not (Test-Path $sourcePath)) {
        continue
    }

    $targetPath = Join-Path $payloadRoot $entry
    $targetDir = Split-Path $targetPath -Parent
    New-Item -ItemType Directory -Force -Path $targetDir | Out-Null
    Copy-Item -LiteralPath $sourcePath -Destination $targetPath -Force
    $stagedFiles += $entry
}

$driverBinary = Join-Path $RepoRoot "build\igd9x.drv"
if (Test-Path $driverBinary) {
    Copy-Item -LiteralPath $driverBinary -Destination (Join-Path $payloadRoot "igd9x.drv") -Force
    $stagedFiles += "build\igd9x.drv"
}

$driverMiniVxd = Join-Path $RepoRoot "build\igd9x.vxd"
if (Test-Path $driverMiniVxd) {
    Copy-Item -LiteralPath $driverMiniVxd -Destination (Join-Path $payloadRoot "igd9x.vxd") -Force
    $stagedFiles += "build\igd9x.vxd"
}

$manifestLines = @()
$manifestLines += "IGD9X Win9x staging manifest"
$manifestLines += "RepoRoot=$RepoRoot"
$manifestLines += "OutputRoot=$OutputRoot"
$manifestLines += ""
$manifestLines += "Staged files:"
foreach ($file in ($stagedFiles | Sort-Object -Unique)) {
    $manifestLines += "  $file"
}

$manifestLines | Set-Content -LiteralPath $manifestPath -Encoding ASCII
Write-Host "Staged package payload to $payloadRoot"
Write-Host "Wrote manifest to $manifestPath"
