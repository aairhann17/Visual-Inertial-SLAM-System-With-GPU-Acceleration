param(
    [switch]$InstallDeps,
    [switch]$NoServerLaunch
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$RepoRoot = Split-Path -Parent $PSScriptRoot
$ApiDir = Join-Path $RepoRoot "services\api"
$WebDir = Join-Path $RepoRoot "apps\web"
$ApiEnv = Join-Path $ApiDir ".env"
$WebEnv = Join-Path $WebDir ".env"
$VislamExe = Join-Path $RepoRoot "build\Debug\vislam_app.exe"

Write-Host "Repository root: $RepoRoot"

if (-not (Test-Path $VislamExe)) {
    throw "vislam_app.exe not found at $VislamExe. Build C++ first: cmake --build build --config Debug"
}

Write-Host "Checking Docker daemon..."
docker info *> $null
if ($LASTEXITCODE -ne 0) {
    throw "Docker daemon is not running. Start Docker Desktop and retry."
}

Write-Host "Starting PostgreSQL container..."
docker compose -f (Join-Path $RepoRoot "docker-compose.yml") up -d postgres | Out-Null

if (-not (Test-Path $ApiEnv)) {
    Copy-Item (Join-Path $ApiDir ".env.example") $ApiEnv
    Write-Host "Created $ApiEnv from .env.example"
}
if (-not (Test-Path $WebEnv)) {
    Copy-Item (Join-Path $WebDir ".env.example") $WebEnv
    Write-Host "Created $WebEnv from .env.example"
}

if ($InstallDeps -or -not (Test-Path (Join-Path $ApiDir "node_modules"))) {
    Write-Host "Installing API dependencies..."
    Push-Location $ApiDir
    try {
        npm install
    } finally {
        Pop-Location
    }
}

if ($InstallDeps -or -not (Test-Path (Join-Path $WebDir "node_modules"))) {
    Write-Host "Installing web dependencies..."
    Push-Location $WebDir
    try {
        npm install
    } finally {
        Pop-Location
    }
}

Write-Host "Initializing PostgreSQL schema..."
Push-Location $ApiDir
try {
    npm run db:init
} finally {
    Pop-Location
}

if ($NoServerLaunch) {
    Write-Host "Setup complete. Server launch skipped by -NoServerLaunch."
    exit 0
}

Write-Host "Starting API server in a new terminal..."
Start-Process powershell -ArgumentList "-NoExit", "-Command", "Set-Location '$ApiDir'; npm run start"

Write-Host "Starting web server in a new terminal..."
Start-Process powershell -ArgumentList "-NoExit", "-Command", "Set-Location '$WebDir'; npm run dev -- --host 127.0.0.1 --port 5173"

Write-Host ""
Write-Host "Done."
Write-Host "API health:  http://127.0.0.1:8080/api/health"
Write-Host "Dashboard:   http://127.0.0.1:5173"
Write-Host ""
Write-Host "Tip: Use -InstallDeps to force npm install, or -NoServerLaunch for setup-only."

