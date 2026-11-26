# SPDX-License-Identifier: GPL-3.0-only
# Run from an x64 Visual Studio Developer PowerShell with Qt and vcpkg available.
[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$build = Join-Path $root 'build/distribution-windows'
$output = Join-Path $root 'build/exports'
$clientId = '8e8a2428-a727-4668-98ea-2fdaf0037943'

function Invoke-Native([string]$Program, [string[]]$Arguments) {
    & $Program @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "$Program failed with exit code $LASTEXITCODE"
    }
}

if ($env:VSCMD_ARG_TGT_ARCH -ne 'x64') {
    throw 'Open the x64 Visual Studio Developer PowerShell before exporting.'
}
if (-not $env:VCPKG_ROOT) {
    throw 'Set VCPKG_ROOT to the x64 vcpkg installation with the launcher dependencies.'
}

New-Item -ItemType Directory -Force -Path $output | Out-Null
$stage = Join-Path $output ('windows-stage-' + [guid]::NewGuid().ToString('N'))
Push-Location $root
$previousPlatform = $env:QT_QPA_PLATFORM
try {
    Invoke-Native cmake @('--preset', 'windows_msvc', '-B', $build,
        '-DVCPKG_TARGET_TRIPLET=x64-windows', "-DLauncher_MSA_CLIENT_ID=$clientId",
        '-DLauncher_BUILD_PLATFORM=sanctuary-windows-x64', '-DBUILD_TESTING=ON')
    Invoke-Native cmake @('--build', $build, '--config', 'Release')
    $env:QT_QPA_PLATFORM = 'offscreen'
    Invoke-Native ctest @('--test-dir', $build, '-C', 'Release', '--output-on-failure')
    Invoke-Native cmake @('--install', $build, '--config', 'Release', '--prefix', $stage)

    $requiredFiles = @('sanctuary-launcher.exe', 'platforms/qwindows.dll',
        'jars/JavaCheck.jar', 'jars/NewLaunch.jar', 'jars/NewLaunchLegacy.jar')
    foreach ($file in $requiredFiles) {
        if (-not (Test-Path -LiteralPath (Join-Path $stage $file) -PathType Leaf)) {
            throw "Missing Windows payload: $file"
        }
    }
    Copy-Item -LiteralPath @('LICENSE', 'COPYING.md') -Destination $stage
    $archive = Join-Path $output 'Sanctuary-Launcher-windows-x64-preview.zip'
    Compress-Archive -Path (Join-Path $stage '*') -DestinationPath $archive -Force
    Get-FileHash -LiteralPath $archive -Algorithm SHA256
}
finally {
    $env:QT_QPA_PLATFORM = $previousPlatform
    Pop-Location
}
