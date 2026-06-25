param(
    [ValidateSet('MT4', 'MT5', 'All')]
    [string]$Target = 'All'
)

$ErrorActionPreference = 'Stop'
$Root = Split-Path -Parent $PSScriptRoot
$Library = Join-Path $Root 'Library'

function Build-Target {
    param(
        [string]$Name,
        [string]$Arch,
        [string]$Generator
    )

    $BuildDir = Join-Path $Library "build\$Arch"
    $BinArch = if ($Arch -eq 'Win32') { 'x86' } else { $Arch }
    $BinDir = Join-Path $Library "bin\$BinArch"

    Write-Host "Building $Name ($Arch)..." -ForegroundColor Cyan

    if (-not (Test-Path $BuildDir)) {
        New-Item -ItemType Directory -Path $BuildDir -Force | Out-Null
    }
    if (-not (Test-Path $BinDir)) {
        New-Item -ItemType Directory -Path $BinDir -Force | Out-Null
    }

    Push-Location $BuildDir
    try {
        cmake -G $Generator -A $Arch $Library
        cmake --build . --config Release
        Write-Host "Output: $BinDir\RmqBridge.dll" -ForegroundColor Green
    }
    finally {
        Pop-Location
    }
}

switch ($Target) {
    'MT4' { Build-Target -Name 'MT4' -Arch 'Win32' -Generator 'Visual Studio 17 2022' }
    'MT5' { Build-Target -Name 'MT5' -Arch 'x64' -Generator 'Visual Studio 17 2022' }
    'All' {
        Build-Target -Name 'MT4' -Arch 'Win32' -Generator 'Visual Studio 17 2022'
        Build-Target -Name 'MT5' -Arch 'x64' -Generator 'Visual Studio 17 2022'
    }
}

Write-Host "Done." -ForegroundColor Green
