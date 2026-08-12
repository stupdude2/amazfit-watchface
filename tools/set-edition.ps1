param(
    [Parameter(Mandatory = $true, Position = 0)]
    [ValidateSet("standard", "pro")]
    [string]$Edition
)

$ErrorActionPreference = "Stop"
$projectRoot = Split-Path -Parent $PSScriptRoot
$packagePath = Join-Path $projectRoot "package.json"
$cEditionPath = Join-Path $projectRoot "src\c\edition.h"
$jsEditionPath = Join-Path $projectRoot "src\pkjs\edition.js"
$utf8NoBom = New-Object System.Text.UTF8Encoding($false)

$standardUuid = "e46902d5-5ef3-4ed2-aad9-fb7deacd56d2"
$proUuid      = "ced51275-d445-4b7e-89f6-9e41110ed4da"

$package = Get-Content $packagePath -Raw | ConvertFrom-Json

if ($Edition -eq "pro") {
    $isPro = $true
    $package.pebble.displayName = "Amazfit Bip Port Pro"
    $package.pebble.uuid = $proUuid
} else {
    $isPro = $false
    $package.pebble.displayName = "Amazfit Bip Port"
    $package.pebble.uuid = $standardUuid
}

$json = $package | ConvertTo-Json -Depth 20
[System.IO.File]::WriteAllText($packagePath, $json + [Environment]::NewLine, $utf8NoBom)

$cValue = if ($isPro) { "1" } else { "0" }
$cText = @"
#pragma once

// Generated/updated by tools/set-edition.ps1.
// 0 = Standard, 1 = Pro
#define WATCHFACE_PRO $cValue
"@
[System.IO.File]::WriteAllText($cEditionPath, $cText + [Environment]::NewLine, $utf8NoBom)

$jsValue = if ($isPro) { "true" } else { "false" }
$jsText = @"
// Generated/updated by tools/set-edition.ps1.
module.exports = {
  isPro: $jsValue
};
"@
[System.IO.File]::WriteAllText($jsEditionPath, $jsText + [Environment]::NewLine, $utf8NoBom)

Write-Host "Edition set to $($Edition.ToUpper())."
Write-Host "Display name: $($package.pebble.displayName)"
Write-Host "UUID: $($package.pebble.uuid)"
