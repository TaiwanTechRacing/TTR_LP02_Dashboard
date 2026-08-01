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

- **Clock setup.** The QSPI timings in `bsp_qspi.c` are derived from HCLK3, so
  the loader configures the same PLL as the firmware. A different clock would
  silently change them.
- **A timebase.** No interrupts run, so there is no SysTick. `HAL_GetTick()` is
  backed by the Cortex-M7 cycle counter instead.
- **Freestanding fill-ins.** Linked `-nostdlib` and without HAL startup, so
  `memset`, `memcpy`, `uwTickPrio`, `HAL_InitTick` and `HAL_GetREVID` are
  supplied locally.
- **MPU off.** The firmware marks the QSPI window no-access until it knows the
  capacity. The loader is the only thing running, so it disables the MPU
  outright rather than reproducing that logic.

## Things that will bite if changed

**`.Dev_Info` must keep its section name.** CubeProgrammer locates the device
descriptor by section, not address. `KEEP()` in the linker script and `used` on
the struct stop `--gc-sections` discarding it.

**Every entry point needs pinning.** Only `Init` is reachable from `ENTRY`, so
`--gc-sections` removes the others. `CMakeLists.txt` passes `--undefined` for
each. A loader missing `Write` fails at the point of writing, not at load time,
which is a confusing way to find out.

**Everything links at a RAM address.** The loader is downloaded into RAM and
called there; there is no flash region and no vector table.

## Status

Builds clean and exports all six entry points with a correct `.Dev_Info`
section. **Not yet run against hardware.**

Before trusting it, run the firmware's own write test - set
`g_qspi_run_write_test` to 1 in a debugger and read `g_qspi_write_test_result`.
That exercises the same erase and program path through the same source file, so
if it fails, the loader would have failed too, and it is far easier to debug
from a running firmware than from inside CubeProgrammer.
