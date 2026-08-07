# W25Q64 External Loader

STM32CubeProgrammer external loader for the QSPI flash on the core board, so
the 8 MB part can be programmed the same way as internal flash.

## Build

```powershell
cd tools\qspi_loader
cmake -S . -B build -G "Ninja" -DCMAKE_TOOLCHAIN_FILE=../../cmake/arm-none-eabi-gcc.cmake
cmake --build build
```

Produces `build/W25Q64_TTR.stldr`.

## Install

Copy it next to the loaders CubeProgrammer ships with:

```
C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\ExternalLoader\
```

It then appears in the External Loaders list in the GUI, or on the command line:

```powershell
STM32_Programmer_CLI -c port=SWD mode=UR -el W25Q64_TTR.stldr -w gifs.bin 0x90000000
```

## The part

Confirmed on hardware by reading the JEDEC ID rather than assumed from the
schematic:

| | |
|---|---|
| JEDEC ID | `0xEF4017` - Winbond W25Q64 |
| Size | 8 MB |
| Mapped at | `0x90000000` |
| Erase unit | 4 KB sector, 2048 of them |
| Program unit | 256-byte page |
| Erased value | `0xFF` |

W25Q parts also support 32 KB and 64 KB block erases, which are faster per byte.
The loader only implements the 4 KB sector erase, and `Dev_Inf.c` describes only
what the loader can actually do - claiming block erases it cannot perform would
make CubeProgrammer issue requests that silently fail.

## How it fits together

The flash operations are **not** reimplemented here. `Core/User/bsp_qspi.c` is
compiled into the loader as-is, so the code that programs the part is the same
code the firmware uses to read it. Two implementations would eventually
disagree, and a loader that writes differently from the firmware that reads is a
bad place for that to hide.

`Loader_Src.c` therefore contains only what a loader needs beyond that:

- **Clock and timebase.** The loader stays on the reset-default 64 MHz HSI, so
  the fixed QSPI divider produces a conservative 16 MHz clock. No interrupts
  run, so `HAL_GetTick()` uses the Cortex-M7 cycle counter.
- **Freestanding fill-ins.** Linked `-nostdlib` and without HAL startup, so
  `memset`, `memcpy` and `HAL_InitTick` are supplied locally.
- **MPU off.** The firmware marks the QSPI window no-access until it knows the
  capacity. The loader is the only thing running, so it disables the MPU
  outright rather than reproducing that logic.

## Things that will bite if changed

**`.Dev_Info` must be a loadable segment at address zero, and code must start at
`0x24000004`.** This is CubeProgrammer's external-loader ELF layout. `KEEP()` in
the linker script and `used` on the struct stop `--gc-sections` discarding the
descriptor.

**Do not call the CMSIS cache-disable helpers from `Init()`.** CubeProgrammer
enters the loader directly without running `Reset_Handler`/`SystemInit`; the
set/way cache-maintenance routine stalled on hardware. The caches are already
disabled by CubeProgrammer's reset.

**Every entry point needs pinning.** Only `Init` is reachable from `ENTRY`, so
`--gc-sections` removes the others. `CMakeLists.txt` passes `--undefined` for
each. A loader missing `Write` fails at the point of writing, not at load time,
which is a confusing way to find out.

**Everything links at a RAM address.** The loader is downloaded into RAM and
called there; there is no flash region and no vector table.

## Status

Verified on hardware with STM32CubeProgrammer 2.20.0 and ST-Link on 2026-08-02:

- Firmware write self-test returned `g_qspi_write_test_result = 1`.
- The loader erased sectors 2044-2047, programmed a 13.35 KB binary at
  `0x907FC000`, and CubeProgrammer verified the complete readback successfully.
- The complete 3.64 MB GIF image was programmed to sectors 0-931 and verified.
  The running firmware detected the `TTRQ` header and created all three LVGL
  GIF objects from the memory-mapped data.
