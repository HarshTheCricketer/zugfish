<#
.SYNOPSIS
    Zugfish - Modern Chess.com-Style Game Reviewer & Analysis Suite
    Universal Installation Script for Windows (PowerShell)

.DESCRIPTION
    Builds and installs Zugfish (zugfish-qt GUI and zugfish-cli) natively on Windows.
    Optionally bundles Qt dependencies with windeployqt and creates desktop shortcuts.

.PARAMETER InstallDir
    Target directory to install Zugfish binaries (default: %LOCALAPPDATA%\Programs\Zugfish).

.PARAMETER NoDeps
    Skip dependency verification and winget prompts.

.PARAMETER CliOnly
    Build and install only the zugfish-cli executable.

.PARAMETER GuiOnly
    Build and install only the zugfish-qt GUI executable.

.EXAMPLE
    .\install.ps1
    .\install.ps1 -InstallDir "C:\Tools\Zugfish"
#>

[CmdletBinding()]
param (
    [string]$InstallDir = "$env:LOCALAPPDATA\Programs\Zugfish",
    [switch]$NoDeps,
    [switch]$CliOnly,
    [switch]$GuiOnly,
    [string]$Generator = ""
)

$ErrorActionPreference = "Stop"

function Write-Info    { Write-Host "[zugfish] $args" -ForegroundColor Cyan }
Write-Host ""
function Write-Success { Write-Host "[zugfish] $args" -ForegroundColor Green }
function Write-Warn    { Write-Host "[zugfish:warn] $args" -ForegroundColor Yellow }
function Write-Err     { Write-Host "[zugfish:error] $args" -ForegroundColor Red }

Write-Host "======================================================" -ForegroundColor Cyan
Write-Host "            Zugfish Windows Installer                 " -ForegroundColor Cyan
Write-Host "======================================================" -ForegroundColor Cyan
Write-Info "Target Installation Directory: $InstallDir"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $ScriptDir

# 1. Dependency Checks
if (-not $NoDeps) {
    Write-Info "Checking build dependencies..."

    # Check CMake
    if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
        Write-Warn "CMake not found."
        if (Get-Command winget -ErrorAction SilentlyContinue) {
            Write-Info "Attempting to install CMake via winget..."
            winget install --id Kitware.CMake -e --accept-source-agreements --accept-package-agreements
            $env:Path = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")
        } else {
            Write-Err "Please install CMake from https://cmake.org/download/ and add it to your PATH."
            exit 1
        }
    } else {
        Write-Success "Found CMake: $(cmake --version | Select-Object -First 1)"
    }

    # Check Stockfish
    if (-not (Get-Command stockfish -ErrorAction SilentlyContinue) -and -not (Get-Command stockfish.exe -ErrorAction SilentlyContinue)) {
        Write-Warn "Stockfish engine not found on PATH."
        if (Get-Command winget -ErrorAction SilentlyContinue) {
            Write-Info "You can install Stockfish via winget: 'winget install Stockfish.Stockfish'"
        } else {
            Write-Warn "Download Stockfish from https://stockfishchess.org/download/ and place stockfish.exe in your PATH."
        }
    } else {
        Write-Success "Found Stockfish engine."
    }
}

# 2. Build Configuration
Write-Info "Configuring CMake build..."

$BuildGuiFlag = if ($CliOnly) { "OFF" } else { "ON" }
$BuildCliFlag = if ($GuiOnly) { "OFF" } else { "ON" }

$CmakeArgs = @(
    "-B", "build",
    "-DCMAKE_BUILD_TYPE=Release",
    "-DBUILD_GUI=$BuildGuiFlag",
    "-DBUILD_CLI=$BuildCliFlag"
)

# Pick generator
if ($Generator -ne "") {
    $CmakeArgs += @("-G", $Generator)
} elseif (Get-Command ninja -ErrorAction SilentlyContinue) {
    $CmakeArgs += @("-G", "Ninja")
}

& cmake @CmakeArgs
if ($LASTEXITCODE -ne 0) {
    Write-Err "CMake configuration failed."
    exit $LASTEXITCODE
}

# 3. Compile
Write-Info "Building Zugfish (Release)..."
& cmake --build build --config Release --parallel
if ($LASTEXITCODE -ne 0) {
    Write-Err "Build failed."
    exit $LASTEXITCODE
}

# 4. Installation
Write-Info "Installing binaries to $InstallDir..."
if (-not (Test-Path $InstallDir)) {
    New-Item -ItemType Directory -Path $InstallDir -Force | Out-Null
}

# Resolve output locations (Visual Studio puts files in build/Release or build/gui/Release; Ninja in build/ and build/gui/)
$CliBinary = $null
$GuiBinary = $null

$PossibleCli = @("build\zugfish-cli.exe", "build\Release\zugfish-cli.exe", "build\cli\Release\zugfish-cli.exe")
foreach ($path in $PossibleCli) {
    if (Test-Path $path) { $CliBinary = $path; break }
}

$PossibleGui = @("build\gui\zugfish-qt.exe", "build\gui\Release\zugfish-qt.exe", "build\Release\zugfish-qt.exe")
foreach ($path in $PossibleGui) {
    if (Test-Path $path) { $GuiBinary = $path; break }
}

if (-not $CliOnly -and $GuiBinary) {
    Copy-Item $GuiBinary -Destination "$InstallDir\zugfish-qt.exe" -Force
    Copy-Item $GuiBinary -Destination "$InstallDir\zugfish.exe" -Force
    Write-Success "Installed GUI: $InstallDir\zugfish-qt.exe"

    # Deploy Qt libraries with windeployqt if present
    if (Get-Command windeployqt -ErrorAction SilentlyContinue) {
        Write-Info "Running windeployqt to bundle runtime dependencies..."
        windeployqt --qmldir gui\qml "$InstallDir\zugfish-qt.exe"
    }

    # Create Desktop and Start Menu Shortcuts
    try {
        $WshShell = New-Object -ComObject WScript.Shell
        
        # Desktop Shortcut
        $DesktopPath = [Environment]::GetFolderPath("Desktop")
        $Shortcut = $WshShell.CreateShortcut("$DesktopPath\Zugfish.lnk")
        $Shortcut.TargetPath = "$InstallDir\zugfish-qt.exe"
        $Shortcut.WorkingDirectory = "$InstallDir"
        $Shortcut.Description = "Zugfish Chess Reviewer"
        $Shortcut.Save()
        Write-Success "Created Desktop Shortcut: $DesktopPath\Zugfish.lnk"

        # Start Menu Shortcut
        $ProgramsPath = [Environment]::GetFolderPath("Programs")
        $StartShortcut = $WshShell.CreateShortcut("$ProgramsPath\Zugfish.lnk")
        $StartShortcut.TargetPath = "$InstallDir\zugfish-qt.exe"
        $StartShortcut.WorkingDirectory = "$InstallDir"
        $StartShortcut.Description = "Zugfish Chess Reviewer"
        $StartShortcut.Save()
        Write-Success "Created Start Menu Shortcut"
    } catch {
        Write-Warn "Could not create shortcuts: $_"
    }
}

if (-not $GuiOnly -and $CliBinary) {
    Copy-Item $CliBinary -Destination "$InstallDir\zugfish-cli.exe" -Force
    Write-Success "Installed CLI: $InstallDir\zugfish-cli.exe"
}

# 5. Check PATH
$UserPath = [System.Environment]::GetEnvironmentVariable("Path", "User")
if ($UserPath -notlike "*$InstallDir*") {
    Write-Info "Adding $InstallDir to User PATH..."
    [System.Environment]::SetEnvironmentVariable("Path", "$UserPath;$InstallDir", "User")
    Write-Success "Added $InstallDir to User PATH (restart terminal to take effect)."
}

Write-Host ""
Write-Host "======================================================" -ForegroundColor Green
Write-Host "     Zugfish has been successfully installed!         " -ForegroundColor Green
Write-Host "======================================================" -ForegroundColor Green
Write-Host ""
Write-Host "You can now run:"
if (-not $CliOnly) {
    Write-Host "  • GUI:  zugfish (or launch from Desktop/Start Menu)" -ForegroundColor Cyan
}
if (-not $GuiOnly) {
    Write-Host "  • CLI:  zugfish-cli --pgn path\to\game.pgn" -ForegroundColor Cyan
}
Write-Host ""
