# PC Simulator

Runs the dashboard UI in a desktop window so layout, fonts, colours and the
stale-signal behaviour can be iterated on without a flash cycle.

## Build and run

```powershell
cmake -S sim -B sim/build -G "MinGW Makefiles"
cmake --build sim/build
sim\build\dashboard_sim.exe
```

Requires a host GCC on `PATH` (TDM-GCC or any MinGW-w64) and CMake. No SDL or
other external dependency - LVGL's Win32 backend is used, which is part of the
LVGL submodule already.

## What it actually runs

The same code as the car:

| | source |
|---|---|
| screens, styles, fonts | `Core/User/ui/` (EEZ output, verbatim) |
| value formatting, stale handling, colours | `Core/User/ui_bind.c` (verbatim) |
| data model | `Core/User/vehicle_data.c` (verbatim) |

Only three things are swapped:

- **Display** - a Win32 window instead of LTDC
- **Data** - a synthetic generator in `sim_main.c` instead of CAN
- **`HAL_GetTick()`** - backed by `GetTickCount()`, declared in `shim/stm32h7xx_hal.h`

The shim directory is placed first on the include path so `ui_bind.c` and
`vehicle_data.c` compile for the host without a single `#ifdef`. If the
simulator needed its own copies of those files it would stop being evidence
about the firmware.

## What the generator does

Values sweep rather than sit still, so clipping, digit width and font coverage
all show up. Speed runs the full 0..250 range, voltages and SOC drift, and the
ready flag toggles between `READY` and `N-RDY`.

Every 12 seconds it stops refreshing the signal groups for 3 seconds. That
exercises the stale path - readings should fall back to `---` and the RTD label
should turn red. This is the behaviour that is hardest to check on the car,
because doing so means unplugging CAN while driving.

## What it cannot tell you

Frame rate, memory pressure, and anything involving SDRAM, cache or LTDC
timing. Those are properties of the hardware. A layout that looks right here
still needs one flash to confirm.

## Relationship to `lv_conf.h`

The simulator shares `Drivers/lv_conf.h` with the firmware so the two cannot
drift apart. Three settings differ, each guarded by `LV_SIMULATOR`:

| setting | firmware | simulator |
|---|---|---|
| `LV_MEM_ADR` | `0xC0100000` (SDRAM) | `0` (normal allocation) |
| `LV_USE_OS` | `LV_OS_NONE` | `LV_OS_WINDOWS` |
| `LV_USE_WINDOWS` | `0` | `1` |

Everything else - colour depth, refresh period, font settings - is identical.
