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
| page order, buttons, debounce | `Core/User/nav.c` (verbatim) |
| animations from QSPI | `Core/User/gif_pages.c` (verbatim) |

Only four things are swapped:

- **Display** - a Win32 window instead of LTDC
- **Data** - a synthetic generator in `sim_data.c` instead of CAN
- **Buttons** - arrow keys instead of GPIO
- **QSPI flash** - a `.bin` read into memory by `sim_qspi.c` instead of the
  memory-mapped window
- **`HAL_GetTick()`** - backed by `GetTickCount()`, declared in `shim/stm32h7xx_hal.h`

## Buttons

| key | dashboard button |
|---|---|
| left arrow | button 1, page back |
| right arrow | button 2, page forward |
| both held 1 s | toggle the FPS overlay |

The keys are read as held-or-not, not as key events, and fed to the same
`Nav_Scan()` the firmware calls at the same 5 ms period. Debounce, the wrap at
both ends of the page list and the one-second hold therefore behave exactly as
they do in the car - a tap shorter than 25 ms is ignored here too.

## Animations

The GIFs live on the QSPI flash, so the simulator needs the same image the
board is programmed with. It looks for one automatically, in order:

1. `$env:TTR_QSPI_IMAGE`, if set
2. `qspi.bin` in the working directory
3. `qspi.bin` at the repository root
4. `dashboard_layout\gif\qspi.bin`

so normally there is nothing to configure:

```powershell
python tools\make_qspi_image.py dashboard_layout\gif\optimized -o qspi.bin
sim\build\dashboard_sim.exe
```

It prints which file it loaded, and if it finds none it says where it looked.
Without an image the simulator behaves like an unprogrammed part, which is also
worth checking - the firmware is required to boot that way.

**The animations are on the debug pages**, so press the right arrow to reach
them. Starting on main and seeing no animation is the expected view.

`gif_pages.c` reaches the flash through `BSP_QSPI_GetMappedBase()`, which
`sim_qspi.c` answers with the loaded file. That indirection is the only change
the feature needed to become testable on a desktop.

## Screenshots

```powershell
sim\build\dashboard_shot.exe out.bmp 6000 5
```

Renders headless at a given moment (ms) on a given page, indexed the same as
the page list in `nav.c` - 5, 6 and 7 are the debug pages. Useful over a remote
session and for catching layout regressions without anyone looking at a screen.

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
