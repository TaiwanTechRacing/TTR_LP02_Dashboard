$ErrorActionPreference = "Stop"

$workspace = Split-Path -Parent $PSScriptRoot
$elfPath = Join-Path $workspace "build\debug\DashBoard_re_1010.elf"

if (-not (Test-Path -LiteralPath $elfPath -PathType Leaf)) {
    Write-Error "ELF not found: $elfPath. Run CMake: build debug first."
    exit 1
}

$candidateDirs = @()
if ($env:STM32CUBEPROGRAMMER_PATH) {
    $candidateDirs += $env:STM32CUBEPROGRAMMER_PATH
}

$pathCommand = Get-Command STM32_Programmer_CLI.exe -ErrorAction SilentlyContinue
if ($pathCommand) {
    $candidateDirs += $pathCommand.Source
}

$candidateDirs += @(
    "C:\ST\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe",
    "C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe"
)

$cubeIdeRoots = Get-ChildItem -Path "C:\ST" -Directory -Filter "STM32CubeIDE_*" -ErrorAction SilentlyContinue
foreach ($root in $cubeIdeRoots) {
    $candidateDirs += Get-ChildItem -Path $root.FullName -Recurse -Filter "STM32_Programmer_CLI.exe" -ErrorAction SilentlyContinue |
        Select-Object -ExpandProperty FullName
}

$programmer = $null
foreach ($candidate in $candidateDirs) {
    if (Test-Path -LiteralPath $candidate -PathType Leaf) {
        $programmer = (Resolve-Path -LiteralPath $candidate).Path
        break
    }

    $candidateExe = Join-Path $candidate "STM32_Programmer_CLI.exe"
    if (Test-Path -LiteralPath $candidateExe -PathType Leaf) {
        $programmer = (Resolve-Path -LiteralPath $candidateExe).Path
        break
    }
}

if (-not $programmer) {
    Write-Error "STM32_Programmer_CLI.exe not found. Add it to PATH or set STM32CUBEPROGRAMMER_PATH."
    exit 1
}

& $programmer -c port=SWD mode=UR freq=24000 -w $elfPath -rst
exit $LASTEXITCODE
