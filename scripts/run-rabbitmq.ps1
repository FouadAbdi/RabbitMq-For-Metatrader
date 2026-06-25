param(
    [ValidateSet('up', 'down', 'logs', 'ps')]
    [string]$Action = 'up'
)

$ErrorActionPreference = 'Stop'
$Root = Split-Path -Parent $PSScriptRoot
$Infra = Join-Path $Root 'infrastructure'

function ConvertTo-WslPath {
    param([string]$Path)
    $resolved = (Resolve-Path -LiteralPath $Path).ProviderPath
    if ($resolved -match '^([A-Za-z]):\\(.*)$') {
        $drive = $Matches[1].ToLower()
        $rest = $Matches[2] -replace '\\', '/'
        return "/mnt/$drive/$rest"
    }
    $wslPath = & wsl wslpath -a $resolved 2>$null
    if ($wslPath) {
        return $wslPath.ToString().Trim()
    }
    throw "Could not convert path to WSL: $Path"
}

if (-not (Get-Command wsl -ErrorAction SilentlyContinue)) {
    Write-Error 'WSL is not installed. Install WSL2 and Docker in WSL — see infrastructure/README.md'
}

$InfraWsl = ConvertTo-WslPath $Infra

& wsl -u root -e bash -lc 'service docker start >/dev/null 2>&1 || true'
& wsl -e bash -lc 'for i in $(seq 1 30); do docker info >/dev/null 2>&1 && exit 0; sleep 1; done; exit 1'
if ($LASTEXITCODE -ne 0) {
    Write-Error 'Docker is not available in WSL. Install Docker in WSL2 or start Docker Desktop with WSL integration enabled. See infrastructure/README.md'
}

Write-Host "Using WSL Docker: $InfraWsl" -ForegroundColor Cyan

$bashCmd = 'cd ' + "'" + $InfraWsl + "'" + ' && bash run.sh ' + $Action
& wsl -e bash -lc $bashCmd

if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

Write-Host 'Verifying AMQP port from Windows...' -ForegroundColor Cyan
for ($i = 1; $i -le 30; $i++) {
    $ok = $false
    try {
        $client = New-Object System.Net.Sockets.TcpClient
        $client.Connect('127.0.0.1', 5672)
        $ok = $client.Connected
        $client.Close()
    } catch {
        $ok = $false
    }
    if ($ok) {
        Write-Host 'AMQP reachable at 127.0.0.1:5672' -ForegroundColor Green
        break
    }
    if ($i -eq 30) {
        Write-Error 'RabbitMQ is not reachable from Windows on 127.0.0.1:5672. Ensure WSL is running and try: wsl -u root service docker start'
    }
    Start-Sleep -Seconds 2
}
