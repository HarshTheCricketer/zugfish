<#
.SYNOPSIS
    Zugfish - Modern Chess.com-Style Game Reviewer & Analysis Suite
    Universal Installation Script for Windows (PowerShell)

.DESCRIPTION
    Builds and installs Zugfish (zugfish-qt GUI and/or zugfish-cli) natively on Windows.
    Automatically detects Visual Studio, MinGW, Qt 6, and Stockfish.

.PARAMETER InstallDir
    Target directory to install Zugfish binaries (default: %LOCALAPPDATA%\Programs\Zugfish).

.PARAMETER QtPath
    Path to your Qt 6 installation (e.g. C:\Qt\6.8.0\msvc2022_64).
    If omitted, the script auto-detects Qt 6 in standard locations.

.PARAMETER NoDeps
    Skip dependency checking and winget prompts.

.PARAMETER CliOnly
    Build and install only the zugfish-cli executable (does not require Qt 6).

.PARAMETER GuiOnly
    Build and install only the zugfish-qt GUI executable.

.PARAMETER Generator
    Explicit CMake generator (e.g. "Visual Studio 17 2022", "Ninja").

.EXAMPLE
    # Recommended one-liner in PowerShell:
    powershell -ExecutionPolicy Bypass -File .\install.ps1

    # Specify Qt location:
    .\install.ps1 -QtPath "C:\Qt\6.8.0\msvc2022_64"

    # Build CLI only (no Qt required):
    .\install.ps1 -CliOnly
#>

[CmdletBinding()]
param (
    [string]$InstallDir = "$env:LOCALAPPDATA\Programs\Zugfish",
    [string]$QtPath = "",
    [switch]$NoDeps,
    [switch]$CliOnly,
    [switch]$GuiOnly,
    [string]$Generator = ""
)

# Do not use $ErrorActionPreference = "Stop" as native tool stderr output can abort scripts in PS 7+
$ErrorActionPreference = "Continue"

function Write-Info    { Write-Host "[zugfish] $args" -ForegroundColor Cyan }
function Write-Success { Write-Host "[zugfish] $args" -ForegroundColor Green }
function Write-Warn    { Write-Host "[zugfish:warn] $args" -ForegroundColor Yellow }
function Write-Err     { Write-Host "[zugfish:error] $args" -ForegroundColor Red }

Write-Host ""
Write-Host "======================================================" -ForegroundColor Cyan
Write-Host "            Zugfish Windows Installer                 " -ForegroundColor Cyan
Write-Host "======================================================" -ForegroundColor Cyan
Write-Info "Target Installation Directory: $InstallDir"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $ScriptDir

# -----------------------------------------------------------------------------
# 1. Dependency Checks
# -----------------------------------------------------------------------------
if (-not $NoDeps) {
    Write-Info "Checking build dependencies..."

    # Check CMake
    $hasCmake = Get-Command cmake -ErrorAction SilentlyContinue
    if (-not $hasCmake) {
        Write-Warn "CMake not found on PATH."
        if (Get-Command winget -ErrorAction SilentlyContinue) {
            Write-Info "Attempting to install CMake via winget..."
            try {
                winget install --id Kitware.CMake -e --accept-source-agreements --accept-package-agreements
                $env:Path = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")
            } catch {
                Write-Warn "winget install failed: $_"
            }
        }
        if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
            Write-Err "CMake is required. Download it from: https://cmake.org/download/"
            exit 1
        }
    } else {
        $cmakeVer = (& cmake --version 2>&1 | Select-Object -First 1)
        Write-Success "Found CMake: $cmakeVer"
    }

    # Check Stockfish
    $hasSf = (Get-Command stockfish -ErrorAction SilentlyContinue) -or (Get-Command stockfish.exe -ErrorAction SilentlyContinue)
    if (-not $hasSf) {
        Write-Warn "Stockfish engine not found on PATH."
        if (Get-Command winget -ErrorAction SilentlyContinue) {
            Write-Info "Attempting to install Stockfish via winget..."
            try {
                winget install --id Stockfish.Stockfish -e --accept-source-agreements --accept-package-agreements
                $env:Path = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")
            } catch {
                Write-Warn "winget install Stockfish failed: $_"
            }
        }
        if (-not (Get-Command stockfish -ErrorAction SilentlyContinue) -and -not (Get-Command stockfish.exe -ErrorAction SilentlyContinue)) {
            Write-Warn "Stockfish is not in PATH. Zugfish can still build, but engine analysis will require configuring Stockfish in Settings."
        } else {
            Write-Success "Found Stockfish engine."
        }
    } else {
        Write-Success "Found Stockfish engine on PATH."
    }
}

# -----------------------------------------------------------------------------
# 2. Qt 6 Detection (for GUI builds)
# -----------------------------------------------------------------------------
$BuildGui = (-not $CliOnly)
$BuildCli = (-not $GuiOnly)

if ($BuildGui) {
    if ($QtPath -eq "") {
        # Check standard environment variables
        if ($env:Qt6_DIR -and (Test-Path $env:Qt6_DIR)) {
            $QtPath = $env:Qt6_DIR
        } elseif ($env:QTDIR -and (Test-Path $env:QTDIR)) {
            $QtPath = $env:QTDIR
        } elseif ($env:CMAKE_PREFIX_PATH -and (Test-Path $env:CMAKE_PREFIX_PATH)) {
            $QtPath = $env:CMAKE_PREFIX_PATH
        } else {
            # Search common Qt installation directories
            $SearchRoots = @(
                "C:\Qt",
                "D:\Qt",
                "$env:SystemDrive\Qt",
                "$env:ProgramFiles\Qt",
                "$env:LOCALAPPDATA\Qt"
            )
            foreach ($root in $SearchRoots) {
                if (Test-Path $root) {
                    $qtVersions = Get-ChildItem -Path $root -Directory -Filter "6.*" -ErrorAction SilentlyContinue |
                                  Sort-Object Name -Descending
                    foreach ($ver in $qtVersions) {
                        $targets = Get-ChildItem -Path $ver.FullName -Directory -ErrorAction SilentlyContinue |
                                   Where-Object { $_.Name -match "msvc2022_64|msvc2019_64|mingw_64|msvc" }
                        if ($targets) {
                            $QtPath = $targets[0].FullName
                            break
                        }
                    }
                }
                if ($QtPath -ne "") { break }
            }
        }
    }

    if ($QtPath -ne "") {
        Write-Success "Detected Qt 6 at: $QtPath"
    } else {
        Write-Warn "Qt 6 was not detected in standard locations."
        Write-Info "If you have Qt 6 installed, run: .\install.ps1 -QtPath `"C:\Qt\6.8.0\msvc2022_64`""
        Write-Info "Or to build only the command-line reviewer without Qt, run: .\install.ps1 -CliOnly"
    }
}

# -----------------------------------------------------------------------------
# 3. Configure CMake
# -----------------------------------------------------------------------------
Write-Info "Configuring CMake project..."

$BuildGuiVal = if ($BuildGui) { "ON" } else { "OFF" }
$BuildCliVal = if ($BuildCli) { "ON" } else { "OFF" }

$CmakeArgs = @(
    "-B", "build",
    "-DCMAKE_BUILD_TYPE=Release",
    "-DBUILD_GUI=$BuildGuiVal",
    "-DBUILD_CLI=$BuildCliVal"
)

if ($QtPath -ne "") {
    $CmakeArgs += @("-DCMAKE_PREFIX_PATH=$QtPath")
}

# Check for vcpkg toolchain
if ($env:VCPKG_ROOT -and (Test-Path "$env:VCPKG_ROOT\scripts\buildsystems\vcpkg.cmake")) {
    $CmakeArgs += @("-DCMAKE_TOOLCHAIN_FILE=$env:VCPKG_ROOT\scripts\buildsystems\vcpkg.cmake")
    Write-Info "Using vcpkg toolchain from: $env:VCPKG_ROOT"
} elseif (Test-Path "C:\vcpkg\scripts\buildsystems\vcpkg.cmake") {
    $CmakeArgs += @("-DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake")
    Write-Info "Using vcpkg toolchain from: C:\vcpkg"
}

# Generator selection:
# Only use Ninja if explicitly requested or if running inside a Developer Prompt where cl.exe or g++ is in PATH.
# Otherwise, let CMake default to Visual Studio generator (works in normal PowerShell).
if ($Generator -ne "") {
    $CmakeArgs += @("-G", $Generator)
} elseif ($env:VSCMD_VER -or (Get-Command cl.exe -ErrorAction SilentlyContinue) -or (Get-Command g++ -ErrorAction SilentlyContinue)) {
    if (Get-Command ninja -ErrorAction SilentlyContinue) {
        $CmakeArgs += @("-G", "Ninja")
    }
}

Write-Info "Running: cmake $($CmakeArgs -join ' ')"
& cmake @CmakeArgs
if ($LASTEXITCODE -ne 0) {
    Write-Err "CMake configuration failed."
    if ($BuildGui -and $QtPath -eq "") {
        Write-Warn "Note: If the error mentions Qt6Config.cmake, specify your Qt 6 directory:"
        Write-Warn "    .\install.ps1 -QtPath `"C:\Qt\6.x.x\msvc2022_64`""
        Write-Warn "Or build only the CLI without Qt:"
        Write-Warn "    .\install.ps1 -CliOnly"
    }
    exit $LASTEXITCODE
}

# -----------------------------------------------------------------------------
# 4. Compile Project
# -----------------------------------------------------------------------------
Write-Info "Compiling Zugfish in Release mode..."
& cmake --build build --config Release --parallel
if ($LASTEXITCODE -ne 0) {
    Write-Err "Compilation failed."
    exit $LASTEXITCODE
}

# -----------------------------------------------------------------------------
# 5. Install Binaries & Bundle Runtime
# -----------------------------------------------------------------------------
Write-Info "Installing binaries to $InstallDir..."
if (-not (Test-Path $InstallDir)) {
    New-Item -ItemType Directory -Path $InstallDir -Force | Out-Null
}

# Locate compiled executables (handles both single-config and multi-config generator layouts)
$CliBinary = $null
$GuiBinary = $null

$PossibleCli = @(
    "build\zugfish-cli.exe",
    "build\Release\zugfish-cli.exe",
    "build\cli\Release\zugfish-cli.exe",
    "build\cli\zugfish-cli.exe"
)
foreach ($p in $PossibleCli) {
    if (Test-Path $p) { $CliBinary = $p; break }
}

$PossibleGui = @(
    "build\gui\zugfish-qt.exe",
    "build\gui\Release\zugfish-qt.exe",
    "build\Release\zugfish-qt.exe",
    "build\zugfish-qt.exe"
)
foreach ($p in $PossibleGui) {
    if (Test-Path $p) { $GuiBinary = $p; break }
}

if ($BuildGui -and $GuiBinary) {
    Copy-Item $GuiBinary -Destination "$InstallDir\zugfish-qt.exe" -Force
    Copy-Item $GuiBinary -Destination "$InstallDir\zugfish.exe" -Force
    Write-Success "Installed GUI: $InstallDir\zugfish-qt.exe"

    # Search for windeployqt to bundle Qt DLLs & QML imports
    $windeployqt = $null
    if ($QtPath -ne "" -and (Test-Path "$QtPath\bin\windeployqt.exe")) {
        $windeployqt = "$QtPath\bin\windeployqt.exe"
    } elseif (Get-Command windeployqt -ErrorAction SilentlyContinue) {
        $windeployqt = "windeployqt"
    }

    if ($windeployqt) {
        Write-Info "Running windeployqt to bundle runtime libraries..."
        & $windeployqt --qmldir gui\qml "$InstallDir\zugfish-qt.exe" 2>&1 | Out-Null
        Write-Success "Bundled Qt runtime libraries into $InstallDir"
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

if ($BuildCli -and $CliBinary) {
    Copy-Item $CliBinary -Destination "$InstallDir\zugfish-cli.exe" -Force
    Write-Success "Installed CLI: $InstallDir\zugfish-cli.exe"
}

# -----------------------------------------------------------------------------
# 6. User PATH Update
# -----------------------------------------------------------------------------
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
if ($BuildGui) {
    Write-Host "  • GUI:  zugfish (or launch from Desktop/Start Menu)" -ForegroundColor Cyan
}
if ($BuildCli) {
    Write-Host "  • CLI:  zugfish-cli --pgn path\to\game.pgn" -ForegroundColor Cyan
}
Write-Host ""
