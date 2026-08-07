$ErrorActionPreference = "Stop"

$workspace = Split-Path -Parent $PSScriptRoot
$iocPath = (Resolve-Path -LiteralPath (Join-Path $workspace "DashBoard_re_1010.ioc")).Path

$candidateDirs = @()
if ($env:STM32CUBEMX_PATH) {
    $candidateDirs += $env:STM32CUBEMX_PATH
}

$pathCommand = Get-Command STM32CubeMX.exe -ErrorAction SilentlyContinue
if ($pathCommand) {
    $candidateDirs += $pathCommand.Source
}

$candidateDirs += @(
    "C:\ST\STM32CubeMX\STM32CubeMX.exe",
    "C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeMX\STM32CubeMX.exe",
    "C:\Program Files\STMicroelectronics\STM32CubeMX\STM32CubeMX.exe",
    "C:\Program Files (x86)\STMicroelectronics\STM32CubeMX\STM32CubeMX.exe"
)

$cubeMx = $null
foreach ($candidate in $candidateDirs) {
    if (Test-Path -LiteralPath $candidate -PathType Leaf) {
        $cubeMx = (Resolve-Path -LiteralPath $candidate).Path
        break
    }

    $candidateExe = Join-Path $candidate "STM32CubeMX.exe"
    if (Test-Path -LiteralPath $candidateExe -PathType Leaf) {
        $cubeMx = (Resolve-Path -LiteralPath $candidateExe).Path
        break
    }
}

if (-not $cubeMx) {
    Write-Error "STM32CubeMX.exe not found. Install standalone STM32CubeMX, add it to PATH, or set STM32CUBEMX_PATH."
    exit 1
}

if (-not (Test-Path -LiteralPath $iocPath)) {
    Write-Error "IOC file not found: $iocPath"
    exit 1
}

Start-Process -FilePath $cubeMx -ArgumentList "`"$iocPath`"" -WorkingDirectory $workspace
